#ifndef NTW16S_TAB_PAINT_H
#define NTW16S_TAB_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/native.h"
/* Private horizontal string/image tabs. Native USER/COMCTL owns all item
 * geometry, page state, keyboard, focus, tooltips and notifications. */
struct classic_tab_colors;
BOOL win16_style_tab_paint(HWND,HDC,const win16_style_palette *,HFONT,const style_instance *,
    const style_control_native_state *,const struct classic_tab_colors *,style_line_profile);
#endif
