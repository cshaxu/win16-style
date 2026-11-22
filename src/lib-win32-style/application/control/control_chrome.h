#ifndef STYLE_CONTROL_CHROME_H
#define STYLE_CONTROL_CHROME_H
#include "../window/palette.h"
#include "control_colors.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
style_relief_palette style_control_outline_palette(const style_window_palette *);
void style_control_outline_scroll_arrow(HDC,RECT,UINT,const style_window_palette *,const style_scroll_arrow_art *);
void style_control_combo_arrow(HDC dc,int left,int top,int dpi,const classic_combo_arrow_art *art);
#endif
