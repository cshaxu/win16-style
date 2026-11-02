#ifndef WIN16_CONTROL_BASIC_PAINT_H
#define WIN16_CONTROL_BASIC_PAINT_H
#include "palette.h"
void win16_control_control_frame(HDC dc,RECT bounds,COLORREF color,int width);
RECT win16_control_button_border(HDC dc,RECT bounds,const win16_style_palette *palette,int unit,int border,BOOL pushed);
void win16_control_scroll_arrow(HDC dc,RECT bounds,const char bits[17][18],const win16_style_palette *palette);
void win16_control_combo_arrow(HDC dc,int left,int top,int dpi);
#endif
