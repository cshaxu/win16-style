#ifndef WIN16_CONTROL_FRAME_LAYOUT_H
#define WIN16_CONTROL_FRAME_LAYOUT_H
#include "layout.h"
BOOL win16_control_measure_window(HWND window,const UINT *dpi,HFONT *menu_font,const BOOL *detached,
    const RECT *outer,win16_style_layout *layout);
#endif
