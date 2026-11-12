#ifndef STYLE_COMMON_FRAME_H
#define STYLE_COMMON_FRAME_H
#include <windows.h>

static inline void style_frame_changed(HWND window)
{
    SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    RedrawWindow(window,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
}
static inline void style_client_screen(HWND window,RECT *rect)
{
    POINT origin={0,0};GetClientRect(window,rect);ClientToScreen(window,&origin);OffsetRect(rect,origin.x,origin.y);
}
static inline void style_restore_client(HWND window,const RECT *wanted)
{
    RECT current,outer;POINT origin;
    style_client_screen(window,&current);GetWindowRect(window,&outer);
    origin.x=outer.left+wanted->left-current.left;origin.y=outer.top+wanted->top-current.top;
    if(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)ScreenToClient(GetParent(window),&origin);
    SetWindowPos(window,NULL,origin.x,origin.y,
        outer.right-outer.left+(wanted->right-wanted->left)-(current.right-current.left),
        outer.bottom-outer.top+(wanted->bottom-wanted->top)-(current.bottom-current.top),SWP_NOZORDER|SWP_NOACTIVATE);
}
#endif
