#ifndef STYLE_TRACKBAR_PAINT_H
#define STYLE_TRACKBAR_PAINT_H
#include "../graphics/paint_frame.h"
#include <commctrl.h>

typedef struct classic_trackbar_colors {
    COLORREF channel,thumb,edge;
    BOOL present;
} classic_trackbar_colors;

typedef enum style_trackbar_profile {
    STYLE_TRACKBAR_DEFAULT,STYLE_TRACKBAR_OUTLINED,STYLE_TRACKBAR_RELIEF
} style_trackbar_profile;

BOOL style_trackbar_paint(HDC,RECT,DWORD,UINT,BOOL,BOOL,BOOL,style_trackbar_profile,
    COLORREF,const style_relief_palette *);
typedef struct style_trackbar_parts {
    const classic_trackbar_colors *axes;
    style_trackbar_profile profile;
    style_relief_palette palette;
    BOOL (*supported)(HWND);
} style_trackbar_parts;

BOOL style_trackbar_bound_supported(HWND);
BOOL style_trackbar_native_supported(HWND);
BOOL style_trackbar_notify_part(HWND,const NMCUSTOMDRAW *,const void *);
BOOL style_trackbar_notify_runtime_border(HWND,const NMCUSTOMDRAW *,const void *);
#endif
