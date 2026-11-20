#ifndef WIN16_CONTROL_BASIC_PAINT_H
#define WIN16_CONTROL_BASIC_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
style_relief_palette win16_control_push_palette(const win16_style_palette *);
void win16_control_scroll_arrow(HDC dc,RECT bounds,const char bits[17][18],const win16_style_palette *palette);
void win16_control_scroll_arrow_art(HDC,RECT,UINT,const win16_style_palette *,const style_scroll_arrow_art *);
void win16_control_combo_arrow(HDC dc,int left,int top,int dpi);
#endif
