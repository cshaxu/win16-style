#include "lib-win32-style/application/menu/tracking.h"
#include <stdio.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"TRACKING line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct observation { unsigned calls;BOOL retire; } observation;
static void observe(style_menu_tracking *frame,MSG *message)
{
    observation *o=frame->context;
    if(message->message==WM_APP+1){++o->calls;if(o->retire)style_menu_tracking_end(frame);}
}
typedef struct foreign { HWND window;style_menu_tracking *frame;BOOL begin_rejected,end_rejected; } foreign;
static DWORD WINAPI other_thread(void *context)
{
    foreign *f=context;style_menu_tracking local={.window=f->window};
    f->begin_rejected=!style_menu_tracking_begin(&local,WH_GETMESSAGE,observe) && GetLastError()==ERROR_INVALID_THREAD_ID;
    f->end_rejected=!style_menu_tracking_end(f->frame) && GetLastError()==ERROR_INVALID_PARAMETER;
    return 0;
}
static BOOL running,idle,timed_out;
static void CALLBACK timeout(HWND w,UINT m,UINT_PTR id,DWORD tick)
{(void)w;(void)m;(void)id;(void)tick;timed_out=TRUE;EndMenu();}
static int menu_hit(const style_menu_tracking *frame,POINT point)
{(void)frame;return point.x==1234 && point.y==5678?1:STYLE_MENU_NO_ITEM;}
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(running && m==WM_ENTERIDLE && a==MSGF_MENU && !idle){
        idle=TRUE;PostMessageW(w,WM_MOUSEMOVE,0,MAKELPARAM(1234,5678));return 0;
    }
    return DefWindowProcW(w,m,a,b);
}
int main(void)
{
    WNDCLASSW cls={.lpfnWndProc=window_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicMenuTracking"};
    CHECK(RegisterClassW(&cls));
    HWND window=CreateWindowW(cls.lpszClassName,L"Owned tracking",WS_OVERLAPPEDWINDOW,20,20,300,180,NULL,NULL,cls.hInstance,NULL);CHECK(window);
    observation first={0},second={0};
    style_menu_tracking a={.window=window,.context=&first},b={.window=window,.context=&second};
    CHECK(!style_menu_tracking_begin(&a,WH_KEYBOARD,observe) && GetLastError()==ERROR_INVALID_PARAMETER);
    CHECK(!style_menu_tracking_current() && style_menu_tracking_begin(&a,WH_GETMESSAGE,observe));
    CHECK(!style_menu_tracking_begin(&a,WH_GETMESSAGE,observe) && GetLastError()==ERROR_ALREADY_EXISTS);
    CHECK(PostMessageW(window,WM_APP+1,0,0));MSG message;
    CHECK(PeekMessageW(&message,window,WM_APP+1,WM_APP+1,PM_NOREMOVE) && first.calls==0);
    CHECK(PeekMessageW(&message,window,WM_APP+1,WM_APP+1,PM_REMOVE) && first.calls==1);
    CHECK(style_menu_tracking_begin(&b,WH_GETMESSAGE,observe));
    CHECK(PostMessageW(window,WM_APP+1,0,0));
    CHECK(PeekMessageW(&message,window,WM_APP+1,WM_APP+1,PM_REMOVE) && first.calls==1 && second.calls==1);
    CHECK(style_menu_tracking_end(&b) && style_menu_tracking_current()==&a);
    CHECK(style_menu_tracking_begin(&b,WH_MSGFILTER,observe));
    message=(MSG){.hwnd=window,.message=WM_APP+1};CallMsgFilterW(&message,MSGF_MENU);
    CHECK(second.calls==2 && first.calls==1);
    foreign f={window,&b,FALSE,FALSE};HANDLE thread=CreateThread(NULL,0,other_thread,&f,0,NULL);CHECK(thread);
    CHECK(WaitForSingleObject(thread,2000)==WAIT_OBJECT_0 && CloseHandle(thread) && f.begin_rejected && f.end_rejected);
    CHECK(style_menu_tracking_end(&a) && style_menu_tracking_current()==&b);
    HMENU root=CreatePopupMenu(),child=CreatePopupMenu();CHECK(root && child);
    b.root=b.active=root;style_menu_tracking_update(window,WM_INITMENUPOPUP,(WPARAM)child,0);CHECK(b.active==child);
    style_menu_tracking_update(window,WM_UNINITMENUPOPUP,(WPARAM)child,0);CHECK(b.active==root);
    CHECK(style_menu_tracking_end(&b) && !style_menu_tracking_current());
    CHECK(style_menu_tracking_begin(&a,WH_GETMESSAGE,observe));
    style_menu_tracking_update(window,WM_MENUSELECT,0,(LPARAM)child);CHECK(a.active==child);
    first.retire=TRUE;CHECK(PostMessageW(window,WM_APP+1,0,0));
    CHECK(PeekMessageW(&message,window,WM_APP+1,WM_APP+1,PM_REMOVE) && first.calls==2 && !style_menu_tracking_current());
    CHECK(AppendMenuW(root,MF_STRING,1,L"Owned item"));
    style_menu_tracking native={.window=window,.root=root,.active=root,.selected=0,.target=STYLE_MENU_NO_ITEM,
        .armed=TRUE,.hit=menu_hit,.navigation=STYLE_MENU_TRACK_NATIVE_POPUPS};
    ShowWindow(window,SW_SHOWNOACTIVATE);
    CHECK(style_menu_tracking_begin(&native,WH_MSGFILTER,style_menu_tracking_input));
    running=TRUE;CHECK(SetTimer(window,99,1500,timeout));
    TrackPopupMenuEx(root,TPM_RETURNCMD|TPM_NOANIMATION,40,80,window,NULL);
    KillTimer(window,99);running=FALSE;
    CHECK(!timed_out && idle && native.target==1 && style_menu_tracking_end(&native) && !style_menu_tracking_current());
    CHECK(DestroyMenu(child) && DestroyMenu(root) && DestroyWindow(window) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("MENU_TRACKING_OK peek=1 single_nested_dispatch=1 mixed=1 non_top=1 thread=1 retire_callback=1 native_input=1");return 0;
}
