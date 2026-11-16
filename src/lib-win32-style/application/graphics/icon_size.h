#ifndef STYLE_ICON_SIZE_H
#define STYLE_ICON_SIZE_H
#include <windows.h>

/* The icon is borrowed; GetIconInfo returns bitmap copies owned here. */
static inline BOOL style_icon_size(HICON icon,int *width,int *height)
{
    ICONINFO info;BITMAP bits;BOOL found=FALSE;
    if(GetIconInfo(icon,&info)){
        if(GetObjectW(info.hbmColor?info.hbmColor:info.hbmMask,sizeof(bits),&bits)){
            *width=bits.bmWidth;*height=info.hbmColor?bits.bmHeight:bits.bmHeight/2;found=TRUE;
        }
        if(info.hbmColor)DeleteObject(info.hbmColor);
        if(info.hbmMask)DeleteObject(info.hbmMask);
    }
    return found;
}
#endif
