#ifndef WIN2K_TRACKBAR_PAINT_H
#define WIN2K_TRACKBAR_PAINT_H
#include <windows.h>
#include <commctrl.h>
#include "../common/palette.h"

typedef struct win2k_trackbar_presentation {
    const classic_trackbar_colors *axes;
    style_trackbar_profile profile;
    const win2k_palette *palette;
} win2k_trackbar_presentation;

BOOL win2k_trackbar_supported(HWND);
BOOL win2k_trackbar_part(HWND,const NMCUSTOMDRAW *,const void *);
#endif
