#ifndef WIN2K_TRACKBAR_PAINT_H
#define WIN2K_TRACKBAR_PAINT_H
#include <windows.h>
#include <commctrl.h>
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

typedef struct win2k_trackbar_presentation {
    const classic_trackbar_colors *axes;
    style_trackbar_profile profile;
} win2k_trackbar_presentation;

BOOL win2k_trackbar_supported(HWND);
BOOL win2k_trackbar_part(HWND,const NMCUSTOMDRAW *,const void *);
#endif
