#ifndef STYLE_PAINT_FRAME_H
#define STYLE_PAINT_FRAME_H
#include <windows.h>

/* Draw an inward solid frame without changing the caller's DC state. */
static inline void style_paint_solid_frame(HDC dc,RECT r,COLORREF color,int width)
{
    HBRUSH brush=CreateSolidBrush(color);
    if(!brush)return;
    for(int i=0;i<width && r.right>r.left && r.bottom>r.top;++i){
        FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);
    }
    DeleteObject(brush);
}
#endif
