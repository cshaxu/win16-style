#ifndef WIN2K_TAB_HEADER_H
#define WIN2K_TAB_HEADER_H
#include "lib-win32-style/application/control/native.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <commctrl.h>

BOOL win2k_tab_supported(HWND);
BOOL win2k_header_supported(HWND);
BOOL win2k_tab_paint(HWND,HDC,HFONT,const style_instance *,const style_control_native_state *,
    const classic_tab_colors *,style_line_profile,const style_relief_palette *);
typedef struct win2k_header_draw {
    const style_instance *instance;
    const classic_header_colors *colors;
    style_relief_palette palette;
    style_line_profile profile;
} win2k_header_draw;
BOOL win2k_header_paint(HWND,const NMCUSTOMDRAW *,const void *);
#endif
