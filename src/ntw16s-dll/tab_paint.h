#ifndef NTW16S_TAB_PAINT_H
#define NTW16S_TAB_PAINT_H
#include "win16_style.h"
/* Private horizontal string/image tabs. Native USER/COMCTL owns all item
 * geometry, page state, keyboard, focus, tooltips and notifications. */
BOOL win16_style_tab_supported(HWND);
BOOL win16_style_tab_paint(HWND,HDC,const win16_style_palette *,HFONT,const BOOL *,const unsigned *);
BOOL win16_style_tab_changes(UINT);
#endif
