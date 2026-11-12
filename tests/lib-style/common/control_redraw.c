#include <windows.h>
#include <commctrl.h>
static LRESULT test_native(HWND,UINT,WPARAM,LPARAM);
#define DefSubclassProc test_native
#include "lib-style/common/control_redraw.h"
#undef DefSubclassProc
static unsigned depth,off_count,on_count;
static BOOL detached,redraw_off,nested,detach_on_call,off_on_call;
static LRESULT test_native(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    (void)lp;
    if(message==WM_SETREDRAW){if(wp)++on_count;else ++off_count;return 0;}
    if(message==WM_APP && !nested){nested=TRUE;
        if(style_control_redraw_call(w,WM_APP+1,0,0,TRUE,&depth,&detached,&redraw_off)!=42)return -1;
        if(detach_on_call)detached=TRUE;
        if(off_on_call)redraw_off=TRUE;
    }
    return 42;
}
int main(void)
{
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,30,30,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent)return 1;
    HWND child=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE,0,0,10,10,parent,NULL,GetModuleHandleW(NULL),NULL);
    if(!child){DestroyWindow(parent);return 2;}
    if(!style_control_redraw_can_suppress(child,FALSE,0) || style_control_redraw_can_suppress(child,TRUE,0))return 3;
    if(style_control_redraw_call(child,WM_APP,0,0,TRUE,&depth,&detached,&redraw_off)!=42 || depth || off_count!=1 || on_count!=1)return 4;
    nested=FALSE;detach_on_call=TRUE;
    if(style_control_redraw_call(child,WM_APP,0,0,TRUE,&depth,&detached,&redraw_off)!=42 || depth || off_count!=2 || on_count!=1)return 5;
    detached=FALSE;detach_on_call=FALSE;off_on_call=TRUE;nested=FALSE;
    if(style_control_redraw_call(child,WM_APP,0,0,TRUE,&depth,&detached,&redraw_off)!=42 || depth || off_count!=3 || on_count!=1)return 6;
    SetWindowLongPtrW(child,GWL_STYLE,WS_CHILD);
    if(style_control_redraw_can_suppress(child,FALSE,0) || !style_control_redraw_can_suppress(child,FALSE,1))return 7;
    DestroyWindow(parent);return 0;
}
