#include "lib-win32/win16-window-uwp/frame.h"
#include "lib-win32/win16-window-uwp/frame_layout.h"
#include "isolated_process.h"
#include <commctrl.h>
#include <windowsx.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static UINT command;
static BOOL tracked_menu;
static void CALLBACK cancel_menu(HWND w,UINT message,UINT_PTR timer,DWORD tick)
{
    (void)message;(void)tick;KillTimer(w,timer);
    GUITHREADINFO info={.cbSize=sizeof(info)};
    tracked_menu=GetGUIThreadInfo(GetCurrentThreadId(),&info) &&
        (info.flags&GUI_INMENUMODE) && info.hwndMenuOwner==win16_uwp_frame_surface(w);
    SendMessageW(w,WM_CANCELMODE,0,0);
}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_SYSCOMMAND){command=(UINT)a&0xfff0;return 0;}
    return DefSubclassProc(w,m,a,b);
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
            tracked_menu=FALSE;CHECK(SetTimer(w,1,200,cancel_menu));
            SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');KillTimer(w,1);
            CHECK(tracked_menu);
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
    CHECK(win16_uwp_frame_apply(w,title));HWND surface=win16_uwp_frame_surface(w);
    CHECK(DestroyWindow(title));CHECK(!IsWindow(surface));CHECK(!win16_uwp_frame_surface(w));
    title=CreateWindowExW(0,L"STATIC",L"Native title",WS_CHILD|WS_VISIBLE,0,0,client.right,48,w,NULL,NULL,NULL);CHECK(title);
    CHECK(win16_uwp_frame_apply(w,title));surface=win16_uwp_frame_surface(w);
    CHECK(DestroyWindow(w));CHECK(!IsWindow(surface));
    puts("UWP_FRAME_LIFECYCLE_PASS private-desktop repeat=3 owner-destroy=1");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
