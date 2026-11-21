#ifndef WIN2K_TAB_HEADER_H
#define WIN2K_TAB_HEADER_H
#include "lib-win32-style/application/control/native.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <commctrl.h>

BOOL win2k_header_supported(HWND);
typedef struct win2k_header_draw {
    const style_instance *instance;
    const style_header_colors *colors;
    style_relief_palette palette;
    style_line_profile profile;
} win2k_header_draw;
BOOL win2k_header_paint(HWND,const NMCUSTOMDRAW *,const void *);
#endif
