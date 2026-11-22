#ifndef STYLE_CONTROL_CHROME_H
#define STYLE_CONTROL_CHROME_H
#include "../window/palette.h"
#include "control_colors.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
style_relief_palette style_control_outline_palette(const style_window_palette *);
void style_control_outline_scroll_arrow(HDC,RECT,UINT,const style_window_palette *,const style_scroll_arrow_art *);
void style_control_combo_arrow(HDC dc,int left,int top,int dpi,const classic_combo_arrow_art *art);
typedef struct style_control_push_appearance {
    classic_push_colors colors;
    style_push_profile profile;
} style_control_push_appearance;
BOOL style_control_bit_button_face(HWND window,HDC dc,const RECT *bounds,const RECT *clip,UINT state,
    const style_window_palette *,const style_control_push_appearance *);
BOOL style_control_push_frame(HWND,HDC,RECT,UINT,const style_window_palette *,const style_control_push_appearance *);
#endif
