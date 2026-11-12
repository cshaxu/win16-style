#ifndef STYLE_COMMON_CLIP_H
#define STYLE_COMMON_CLIP_H
#include <windows.h>

/* Points are in device coordinates. Preserve the caller's DC mapping. */
static inline BOOL style_clip_covers(HDC dc,const POINT points[2])
{
    int saved=SaveDC(dc);if(!saved)return FALSE;
    SetViewportOrgEx(dc,0,0,NULL);SetWindowOrgEx(dc,0,0,NULL);
    RECT bounds;int kind=GetClipBox(dc,&bounds);BOOL covered=FALSE;
    if(kind!=ERROR && kind!=NULLREGION && bounds.left<=points[0].x && bounds.top<=points[0].y &&
       bounds.right>=points[1].x && bounds.bottom>=points[1].y){
        HRGN clip=CreateRectRgn(0,0,0,0);
        if(clip){
            int have=GetClipRgn(dc,clip);
            if(have==0)covered=TRUE;
            else if(have==1){
                HRGN missing=CreateRectRgn(points[0].x,points[0].y,points[1].x,points[1].y);
                if(missing){covered=CombineRgn(missing,missing,clip,RGN_DIFF)==NULLREGION;DeleteObject(missing);}
            }
            DeleteObject(clip);
        }
    }
    RestoreDC(dc,saved);return covered;
}
#endif
