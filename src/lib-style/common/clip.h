#ifndef STYLE_COMMON_CLIP_H
#define STYLE_COMMON_CLIP_H
#include <windows.h>

/* Snapshot owns its region until disposed; kind -1 means capture failed,
 * 0 means the DC has no application clip, 1 means region is valid. */
typedef struct style_clip_snapshot { HRGN region; int kind; } style_clip_snapshot;
static inline void style_clip_snapshot_capture(style_clip_snapshot *snapshot,HDC dc)
{
    snapshot->region=CreateRectRgn(0,0,0,0);snapshot->kind=-1;
    if(snapshot->region)snapshot->kind=GetClipRgn(dc,snapshot->region);
}
static inline BOOL style_clip_snapshot_apply(const style_clip_snapshot *snapshot,HDC dc)
{
    return snapshot->kind>=0 && SelectClipRgn(dc,snapshot->kind?snapshot->region:NULL)!=ERROR;
}
static inline void style_clip_snapshot_dispose(style_clip_snapshot *snapshot)
{
    if(snapshot->region){DWORD error=GetLastError();DeleteObject(snapshot->region);SetLastError(error);}
    snapshot->region=NULL;snapshot->kind=-1;
}

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
