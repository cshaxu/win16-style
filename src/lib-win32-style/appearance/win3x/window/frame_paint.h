#ifndef WIN16_CONTROL_FRAME_PAINT_H
#define WIN16_CONTROL_FRAME_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/window/full_client_layout.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/graphics/font_pair.h"
#include "lib-win32-style/application/window/caption_button.h"
typedef struct win16_frame_paint_state {
    style_font_pair *fonts;
    BOOL active,system_menu,bar_keyboard;
    int bar_selected;
    style_caption_button button;
    const classic_caption_art *art;
    const classic_menu_colors *menu_bar;
    style_caption_presentation presentation;
    BOOL gradient;
    COLORREF end;
    style_border_presentation border;
    const classic_menu_metrics *menu_metrics;
    BOOL close_disabled;
    HICON icon;
} win16_frame_paint_state;
typedef void (*win16_frame_state_read)(void *,win16_frame_paint_state *);
void win16_control_fill(HDC dc,RECT r,COLORREF color);
void win16_control_outline(HDC dc,RECT r,int unit,COLORREF color);
void win16_control_frame_caption(HDC dc,const style_full_client_layout *layout,const win16_style_palette *palette,
    const win16_frame_paint_state *state,PCWSTR title,BOOL child,BOOL zoomed);
#endif
