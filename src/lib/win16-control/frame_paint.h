#ifndef WIN16_CONTROL_FRAME_PAINT_H
#define WIN16_CONTROL_FRAME_PAINT_H
#include "palette.h"
#include "layout.h"
void win16_control_fill(HDC dc,RECT r,COLORREF color);
void win16_control_outline(HDC dc,RECT r,int unit,COLORREF color);
void win16_control_frame_button(HDC dc,RECT r,int kind,BOOL down,BOOL zoom,BOOL child,int unit,
    const win16_style_palette *palette);
void win16_control_frame_sizing(HDC dc,const win16_style_layout *layout,const win16_style_palette *palette);
void win16_control_frame_caption(HWND window,HDC dc,const win16_style_layout *layout,const win16_style_palette *palette,
    const BOOL *active,const BOOL *system_menu,const BOOL *bar_keyboard,const int *bar_selected,
    const int *pressed,const BOOL *hot,HFONT *title_font);
#endif
