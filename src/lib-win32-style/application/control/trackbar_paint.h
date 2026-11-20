#ifndef STYLE_TRACKBAR_PAINT_H
#define STYLE_TRACKBAR_PAINT_H
#include "../graphics/paint_frame.h"

typedef enum style_trackbar_profile {
    STYLE_TRACKBAR_DEFAULT,STYLE_TRACKBAR_OUTLINED,STYLE_TRACKBAR_RELIEF
} style_trackbar_profile;

BOOL style_trackbar_paint(HDC,RECT,DWORD,UINT,BOOL,BOOL,BOOL,style_trackbar_profile,
    COLORREF,const style_relief_palette *);
#endif
