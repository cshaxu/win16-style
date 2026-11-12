#ifndef STYLE_CONTROL_REDRAW_H
#define STYLE_CONTROL_REDRAW_H
#include <windows.h>
#include <commctrl.h>

/* USER makes a window visible on WM_SETREDRAW(TRUE); do not bracket an
 * application-hidden window unless it is already inside our own bracket. */
static inline BOOL style_control_redraw_can_suppress(HWND window,BOOL redraw_off,unsigned depth)
{return !redraw_off && (depth || (GetWindowLongPtrW(window,GWL_STYLE)&WS_VISIBLE));}

/* The caller keeps the referenced state alive across native reentrancy. */
static inline void style_control_redraw_begin(HWND window,unsigned *depth)
{if((*depth)++==0)DefSubclassProc(window,WM_SETREDRAW,FALSE,0);}
static inline void style_control_redraw_end(HWND window,unsigned *depth,const BOOL *detached,const BOOL *redraw_off)
{if(--*depth==0 && !*detached && !*redraw_off)DefSubclassProc(window,WM_SETREDRAW,TRUE,0);}
static inline LRESULT style_control_redraw_call(HWND window,UINT message,WPARAM wp,LPARAM lp,BOOL suppress,
    unsigned *depth,const BOOL *detached,const BOOL *redraw_off)
{
    if(suppress)style_control_redraw_begin(window,depth);
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    if(suppress)style_control_redraw_end(window,depth,detached,redraw_off);
    return result;
}
#endif
