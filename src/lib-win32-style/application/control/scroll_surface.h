#ifndef STYLE_SCROLL_SURFACE_H
#define STYLE_SCROLL_SURFACE_H
#include <windows.h>
#include <oleacc.h>
typedef struct style_scroll_surface {
    SCROLLBARINFO info;
    RECT bar,target;
    POINT origin;
} style_scroll_surface;
/* Native state and coordinate snapshot only. bar is local to the scrollbar;
 * target/origin address its window DC (client DC for OBJID_CLIENT). */
BOOL style_scroll_surface_read(HWND,LONG,style_scroll_surface *);
#endif
