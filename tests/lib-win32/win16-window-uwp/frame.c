#include "lib-win32/win16-window-uwp/frame.h"
#include "lib-win32/win16-window-uwp/frame_layout.h"
#include "isolated_process.h"
#include <commctrl.h>
#include <windowsx.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static UINT command,menu_notifications;
static BOOL tracked_menu,menu_states_ok,expect_action,action_seen;
static unsigned action_calls,expected_actions;
static HWND retiring_title;
static BOOL retired_in_menu;
static void CALLBACK retire_title(HWND w,UINT message,UINT_PTR timer,DWORD tick)
{
    (void)message;(void)tick;KillTimer(w,timer);
    GUITHREADINFO info={.cbSize=sizeof(info)};
    retired_in_menu=GetGUIThreadInfo(GetCurrentThreadId(),&info) &&
        (info.flags&GUI_INMENUMODE) && DestroyWindow(retiring_title);
    if(!retired_in_menu)SendMessageW(w,WM_CANCELMODE,0,0);
}
static void extra_action(HWND frame,void *context)
{(void)frame;(*(unsigned *)context)++;}
static void CALLBACK cancel_menu(HWND w,UINT message,UINT_PTR timer,DWORD tick)
{
    (void)message;(void)tick;KillTimer(w,timer);
    GUITHREADINFO info={.cbSize=sizeof(info)};
    tracked_menu=GetGUIThreadInfo(GetCurrentThreadId(),&info) &&
        (info.flags&GUI_INMENUMODE) && info.hwndMenuOwner==win16_uwp_frame_surface(w);
    HMENU menu=GetSystemMenu(w,FALSE);
    UINT restore=GetMenuState(menu,SC_RESTORE,MF_BYCOMMAND);
    UINT maximize=GetMenuState(menu,SC_MAXIMIZE,MF_BYCOMMAND);
    printf("SYSTEM_MENU_STATE zoomed=%d restore=%x maximize=%x\n",IsZoomed(w),restore,maximize);
    menu_states_ok=restore!=(UINT)-1 && maximize!=(UINT)-1 &&
        !!(restore&(MF_DISABLED|MF_GRAYED))==!IsZoomed(w) &&
        !!(maximize&(MF_DISABLED|MF_GRAYED))==!!IsZoomed(w);
    UINT move=GetMenuState(menu,SC_MOVE,MF_BYCOMMAND),size=GetMenuState(menu,SC_SIZE,MF_BYCOMMAND);
    menu_states_ok=menu_states_ok && !!(move&MF_GRAYED)==!!IsZoomed(w) &&
        !!(size&MF_GRAYED)==!!IsZoomed(w);
    if(expect_action){
        action_seen=GetMenuState(menu,0xefe0,MF_BYCOMMAND)!=(UINT)-1 &&
            GetMenuState(menu,0xeff0,MF_BYCOMMAND)!=(UINT)-1;
        if(expected_actions==2)action_seen=action_seen&&GetMenuState(menu,0xefd0,MF_BYCOMMAND)!=(UINT)-1;
    }
    SendMessageW(w,WM_CANCELMODE,0,0);
}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_INITMENU)++menu_notifications;
    if(m==WM_SYSCOMMAND){command=(UINT)a&0xfff0;return 0;}
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK expanded_maximized_client(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    LRESULT result=DefSubclassProc(w,m,a,b);
    if(m==WM_NCCALCSIZE && a && IsZoomed(w))((NCCALCSIZE_PARAMS *)b)->rgrc[0].bottom+=80;
    return result;
}
static void pump(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}}
static void settle(void)
{
    /* Native minimize/restore can leave transient animation windows after
     * ShowWindow returns. Drain a bounded half-second before resource counts. */
    ULONGLONG end=GetTickCount64()+500;
    do{pump();DWORD left=(DWORD)(end>GetTickCount64()?end-GetTickCount64():0);
        if(!left)break;
        MsgWaitForMultipleObjects(0,NULL,FALSE,left,QS_ALLINPUT);
    }while(GetTickCount64()<end);
    pump();
}
static int worker(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HWND w=CreateWindowExW(0,L"STATIC",L"Frame fixture",WS_OVERLAPPEDWINDOW|WS_VISIBLE,20,20,500,400,NULL,NULL,NULL,NULL);
    CHECK(w);ShowWindow(w,SW_SHOWNOACTIVATE);RECT client,before,after;GetClientRect(w,&client);
    {
        HWND nonclient=CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP,L"STATIC",L"Non-client fixture",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,20,20,500,400,NULL,NULL,NULL,NULL);
        CHECK(nonclient);LONG_PTR original=GetWindowLongPtrW(nonclient,GWL_EXSTYLE);
        CHECK(win16_uwp_frame_apply_nonclient(nonclient));
        HWND nonclient_surface=win16_uwp_frame_surface(nonclient);CHECK(nonclient_surface);
        ShowWindow(nonclient,SW_MAXIMIZE);pump();
        CHECK(SetWindowSubclass(nonclient,expanded_maximized_client,2,0));
        MONITORINFO monitor={.cbSize=sizeof(monitor)};
        CHECK(GetMonitorInfoW(MonitorFromWindow(nonclient,MONITOR_DEFAULTTONEAREST),&monitor));
        CHECK(SetWindowPos(nonclient,NULL,monitor.rcWork.left,monitor.rcWork.top,
            monitor.rcWork.right-monitor.rcWork.left,monitor.rcWork.bottom-monitor.rcWork.top+80,
            SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED));pump();
        CHECK(IsZoomed(nonclient));CHECK(IsWindowVisible(nonclient_surface));
        CHECK(RemoveWindowSubclass(nonclient,expanded_maximized_client,2));
        CHECK(win16_uwp_frame_remove(nonclient));
        CHECK(GetWindowLongPtrW(nonclient,GWL_EXSTYLE)==original);
        CHECK(DestroyWindow(nonclient));
    }
    HWND title=CreateWindowExW(0,L"STATIC",L"Native title",WS_CHILD|WS_VISIBLE,0,0,client.right,48,w,NULL,NULL,NULL);
    CHECK(title);GetWindowRect(w,&before);LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    CHECK(SetWindowSubclass(w,observe,1,0));
    /* Activating a private-desktop window initializes USER/IME helpers
     * asynchronously. Exercise the native path before measuring the adapter. */
    SetForegroundWindow(w);
    for(int i=0;i<2;++i){
        ShowWindow(w,SW_MINIMIZE);ShowWindow(w,SW_RESTORE);
        ShowWindow(w,SW_MAXIMIZE);ShowWindow(w,SW_RESTORE);settle();
    }
    DWORD gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    DWORD user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
    for(int i=0;i<3;++i){
        int left=i?client.right/2:0;
        CHECK(SetWindowPos(title,NULL,left,0,client.right-left,48,SWP_NOZORDER|SWP_NOACTIVATE));
        CHECK(win16_uwp_frame_apply(w,title));HWND surface=win16_uwp_frame_surface(w);CHECK(surface);
        CHECK(!win16_uwp_frame_apply(w,title)&&GetLastError()==ERROR_ALREADY_EXISTS);
        CHECK(GetWindow(surface,GW_OWNER)==w);
        HRGN region=CreateRectRgn(0,0,0,0);CHECK(GetWindowRgn(surface,region)!=ERROR);
        POINT p={10,60};ClientToScreen(w,&p);p.x-=before.left;p.y-=before.top;
        CHECK(!PtInRegion(region,p.x,p.y));DeleteObject(region);
        RECT band;GetWindowRect(title,&band);POINT origin={0,0};ClientToScreen(w,&origin);
        RECT hole={origin.x-before.left,band.bottom-before.top,origin.x-before.left+client.right,origin.y-before.top+client.bottom};
        SIZE size={before.right-before.left,before.bottom-before.top};win16_style_layout layout;
        CHECK(win16_uwp_frame_layout(size,GetDpiForWindow(w),(DWORD)style,FALSE,hole,&layout));
        LPARAM click=MAKELPARAM(layout.minimize.left+2,layout.minimize.top+2);
        command=0;SendMessageW(surface,WM_LBUTTONDOWN,MK_LBUTTON,click);
        SendMessageW(surface,WM_LBUTTONUP,0,click);pump();CHECK(command==SC_MINIMIZE);
        ShowWindow(w,SW_HIDE);CHECK(!IsWindowVisible(surface));ShowWindow(w,SW_SHOWNOACTIVATE);
        CHECK(IsWindowVisible(surface));
        /* Native full-screen modes may temporarily hide the title. Keep the
         * adapter installed, but never leave an input surface over content. */
        ShowWindow(title,SW_HIDE);CHECK(!IsWindowVisible(surface));
        CHECK(win16_uwp_frame_surface(w)==surface);
        ShowWindow(title,SW_SHOWNOACTIVATE);CHECK(IsWindowVisible(surface));
        RECT title_client;CHECK(GetClientRect(w,&title_client));
        CHECK(EqualRect(&client,&title_client));
        HMENU menu=GetSystemMenu(w,FALSE);MENUITEMINFOW native={.cbSize=sizeof(native),.fMask=MIIM_FTYPE|MIIM_DATA};
        CHECK(GetMenuItemInfoW(menu,SC_CLOSE,FALSE,&native));
        SendMessageW(w,WM_ENTERMENULOOP,FALSE,0);
        SendMessageW(w,WM_INITMENUPOPUP,(WPARAM)menu,MAKELPARAM(0,TRUE));
        MENUITEMINFOW styled={.cbSize=sizeof(styled),.fMask=MIIM_FTYPE|MIIM_DATA};
        CHECK(GetMenuItemInfoW(menu,SC_CLOSE,FALSE,&styled));CHECK(styled.fType&MFT_OWNERDRAW);
        MEASUREITEMSTRUCT measure={.CtlType=ODT_MENU,.itemID=SC_CLOSE,.itemData=styled.dwItemData};
        CHECK(SendMessageW(w,WM_MEASUREITEM,0,(LPARAM)&measure));CHECK(measure.itemWidth&&measure.itemHeight);
        SendMessageW(w,WM_EXITMENULOOP,FALSE,0);
        CHECK(GetMenuItemInfoW(menu,SC_CLOSE,FALSE,&styled));
        CHECK(styled.fType==native.fType&&styled.dwItemData==native.dwItemData);
        if(!i){
            tracked_menu=FALSE;menu_notifications=0;CHECK(SetTimer(w,1,200,cancel_menu));
            SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');KillTimer(w,1);
            CHECK(tracked_menu);CHECK(menu_states_ok);CHECK(menu_notifications==1);
            CHECK(GetMenuItemInfoW(menu,SC_CLOSE,FALSE,&styled));
            CHECK(styled.fType==native.fType&&styled.dwItemData==native.dwItemData);
        }
        if(i<2){
            CHECK(SetWindowPos(w,NULL,before.left+20,before.top+20,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE));
            RECT moved,layer;GetWindowRect(w,&moved);GetWindowRect(surface,&layer);CHECK(EqualRect(&moved,&layer));
            ShowWindow(w,SW_MINIMIZE);CHECK(!IsWindowVisible(surface));ShowWindow(w,SW_RESTORE);
            GetClientRect(w,&client);CHECK(SetWindowPos(title,NULL,0,0,client.right,48,SWP_NOZORDER|SWP_NOACTIVATE));
            CHECK(IsWindowVisible(surface));
            ShowWindow(w,SW_MAXIMIZE);GetClientRect(w,&client);
            CHECK(SetWindowPos(title,NULL,0,0,client.right,48,SWP_NOZORDER|SWP_NOACTIVATE));
            GetWindowRect(w,&moved);GetWindowRect(surface,&layer);
            MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT visible;
            CHECK(GetMonitorInfoW(MonitorFromWindow(w,MONITOR_DEFAULTTONEAREST),&monitor));
            CHECK(IntersectRect(&visible,&moved,&monitor.rcWork));
            CHECK(EqualRect(&visible,&layer));CHECK(IsWindowVisible(surface));
            if(!i){
                UINT original=GetMenuState(menu,SC_MAXIMIZE,MF_BYCOMMAND);
                EnableMenuItem(menu,SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);
                CHECK(SetTimer(w,1,200,cancel_menu));
                SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');KillTimer(w,1);
                CHECK(menu_states_ok);
                CHECK(GetMenuState(menu,SC_MAXIMIZE,MF_BYCOMMAND)==original);
                CHECK(GetMenuState(menu,SC_CLOSE,MF_BYCOMMAND)&MF_GRAYED);
                EnableMenuItem(menu,SC_CLOSE,MF_BYCOMMAND|MF_ENABLED);
            }
            ShowWindow(w,SW_RESTORE);GetClientRect(w,&client);
            CHECK(SetWindowPos(title,NULL,0,0,client.right,48,SWP_NOZORDER|SWP_NOACTIVATE));
            CHECK(SetWindowPos(w,NULL,before.left,before.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE));
        }
        CHECK(win16_uwp_frame_remove(w));CHECK(!IsWindow(surface));
        GetWindowRect(w,&after);CHECK(EqualRect(&before,&after));CHECK(GetWindowLongPtrW(w,GWL_STYLE)==style);
        settle();
        /* The first complete menu/adapter cycle also realizes process caches. */
        if(!i){gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);}
        else {DWORD now_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),now_user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
            printf("RESOURCES iteration=%d gdi=%lu/%lu user=%lu/%lu\n",i,now_gdi,gdi,now_user,user);
            CHECK(now_gdi<=gdi);CHECK(now_user<=user);}
    }
    HMENU extras=GetSystemMenu(w,FALSE);
    CHECK(AppendMenuW(extras,MF_STRING,0xeff0,L"Application-owned collision"));
    int count=GetMenuItemCount(extras);
    win16_uwp_frame_action invalid={0};
    CHECK(!win16_uwp_frame_apply_action(w,title,&invalid)&&GetLastError()==ERROR_INVALID_PARAMETER);
    win16_uwp_frame_action action={L"Application menu...",extra_action,&action_calls};
    win16_uwp_frame_action actions[]={action,{L"Second action",extra_action,&action_calls}};
    CHECK(!win16_uwp_frame_apply_actions(w,title,NULL,1)&&GetLastError()==ERROR_INVALID_PARAMETER);
    CHECK(!win16_uwp_frame_apply_actions(w,title,actions,257)&&GetLastError()==ERROR_INVALID_PARAMETER);
    for(int iteration=0;iteration<2;++iteration){
        expected_actions=(unsigned)iteration+1;
        CHECK(win16_uwp_frame_apply_actions(w,title,actions,expected_actions));
        CHECK(!win16_uwp_frame_apply_action(w,title,&action)&&GetLastError()==ERROR_ALREADY_EXISTS);
        CHECK(GetMenuItemCount(extras)==count);
        expect_action=TRUE;action_seen=FALSE;CHECK(SetTimer(w,1,200,cancel_menu));
        SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');KillTimer(w,1);expect_action=FALSE;
        CHECK(action_seen);CHECK(!action_calls);CHECK(GetMenuItemCount(extras)==count);
        CHECK(GetMenuState(extras,0xefe0,MF_BYCOMMAND)==(UINT)-1);
        CHECK(GetMenuState(extras,0xefd0,MF_BYCOMMAND)==(UINT)-1);
        CHECK(GetMenuState(extras,0xeff0,MF_BYCOMMAND)!=(UINT)-1);
        CHECK(win16_uwp_frame_remove(w));CHECK(GetMenuItemCount(extras)==count);
    }
    /* Leave one free command for two actions: partial insertion must unwind. */
    for(UINT id=0xefd0;id>=0xe000;id-=16)CHECK(AppendMenuW(extras,MF_STRING,id,L"Reserved"));
    count=GetMenuItemCount(extras);
    CHECK(win16_uwp_frame_apply_actions(w,title,actions,2));
    SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');
    CHECK(GetMenuItemCount(extras)==count&&!action_calls);
    CHECK(GetMenuState(extras,0xefe0,MF_BYCOMMAND)==(UINT)-1);
    CHECK(win16_uwp_frame_remove(w));
    for(UINT id=0xefd0;id>=0xe000;id-=16)CHECK(DeleteMenu(extras,id,MF_BYCOMMAND));
    CHECK(DeleteMenu(extras,0xeff0,MF_BYCOMMAND));
    /* Two frames on one host thread must have independent lifetimes. Retire
     * one native title inside TrackPopupMenu's nested loop, then reattach it. */
    HWND peer=CreateWindowExW(0,L"STATIC",L"Other hosted app",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        550,20,400,300,NULL,NULL,NULL,NULL);CHECK(peer);ShowWindow(peer,SW_SHOWNOACTIVATE);
    RECT peer_client;CHECK(GetClientRect(peer,&peer_client));
    HWND peer_title=CreateWindowExW(0,L"STATIC",L"Other native title",WS_CHILD|WS_VISIBLE,
        0,0,peer_client.right,48,peer,NULL,NULL,NULL);CHECK(peer_title);
    CHECK(win16_uwp_frame_apply(peer,peer_title));
    HWND peer_surface=win16_uwp_frame_surface(peer);CHECK(peer_surface);
    RECT peer_before,peer_after;CHECK(GetWindowRect(peer,&peer_before));
    CHECK(win16_uwp_frame_apply_actions(w,title,actions,2));
    HWND retiring_surface=win16_uwp_frame_surface(w);CHECK(retiring_surface);
    count=GetMenuItemCount(extras);retiring_title=title;retired_in_menu=FALSE;
    CHECK(SetTimer(w,2,100,retire_title));
    SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');KillTimer(w,2);
    CHECK(retired_in_menu&&!IsWindow(retiring_surface)&&!win16_uwp_frame_surface(w));
    CHECK(GetMenuItemCount(extras)==count&&!action_calls);
    CHECK(win16_uwp_frame_surface(peer)==peer_surface&&IsWindowVisible(peer_surface));
    CHECK(GetWindowRect(peer,&peer_after)&&EqualRect(&peer_before,&peer_after));
    title=CreateWindowExW(0,L"STATIC",L"Replacement title",WS_CHILD|WS_VISIBLE,
        0,0,client.right,48,w,NULL,NULL,NULL);CHECK(title);
    CHECK(win16_uwp_frame_apply(w,title));CHECK(win16_uwp_frame_remove(peer));
    CHECK(!IsWindow(peer_surface)&&win16_uwp_frame_surface(w));
    CHECK(DestroyWindow(peer));CHECK(win16_uwp_frame_remove(w));
    puts("UWP_FRAME_SHARED_THREAD_PASS menu-title-destroy=1 explicit-reattach=1 peer-isolated=1");
    CHECK(win16_uwp_frame_apply(w,title));HWND surface=win16_uwp_frame_surface(w);
    CHECK(DestroyWindow(title));CHECK(!IsWindow(surface));CHECK(!win16_uwp_frame_surface(w));
    title=CreateWindowExW(0,L"STATIC",L"Native title",WS_CHILD|WS_VISIBLE,0,0,client.right,48,w,NULL,NULL,NULL);CHECK(title);
    CHECK(win16_uwp_frame_apply(w,title));surface=win16_uwp_frame_surface(w);
    CHECK(DestroyWindow(w));CHECK(!IsWindow(surface));
    puts("UWP_FRAME_LIFECYCLE_PASS private-desktop repeat=3 owner-destroy=1");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
