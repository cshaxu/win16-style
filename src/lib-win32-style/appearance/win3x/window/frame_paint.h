#ifndef WIN16_CONTROL_FRAME_PAINT_H
#define WIN16_CONTROL_FRAME_PAINT_H
#include "../common/palette.h"
#include "../common/layout.h"
#include "caption_art.h"
#include "lib-win32-style/application/graphics/font_pair.h"
#include "lib-win32-style/application/window/caption_button.h"
typedef struct win16_frame_paint_state {
    style_font_pair *fonts;
    BOOL active,system_menu,bar_keyboard;
    int bar_selected;
    style_caption_button button;
    const win16_caption_art *art;
} win16_frame_paint_state;
typedef void (*win16_frame_state_read)(void *,win16_frame_paint_state *);
void win16_control_fill(HDC dc,RECT r,COLORREF color);
void win16_control_outline(HDC dc,RECT r,int unit,COLORREF color);
void win16_control_frame_sizing(HDC dc,const win16_style_layout *layout,const win16_style_palette *palette);
void win16_control_frame_caption(HDC dc,const win16_style_layout *layout,const win16_style_palette *palette,
    const win16_frame_paint_state *state,PCWSTR title,BOOL child,BOOL zoomed);
#endif
