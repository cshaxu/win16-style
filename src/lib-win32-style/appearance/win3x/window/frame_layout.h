#ifndef WIN16_CONTROL_FRAME_LAYOUT_H
#define WIN16_CONTROL_FRAME_LAYOUT_H
#include "../common/layout.h"
BOOL win16_control_measure_window(HWND window,const UINT *dpi,HFONT *menu_font,const BOOL *detached,
    const RECT *outer,win16_style_layout *layout);
BOOL win16_control_frame_hit_test(HWND window,const win16_style_layout *layout,LPARAM point,LRESULT *result);
#endif
