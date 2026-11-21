#ifndef STYLE_CLIENT_FRAME_PAINT_H
#define STYLE_CLIENT_FRAME_PAINT_H
#include "palette.h"
#include "lib-win32-style/application/window/full_client_layout.h"
#include "caption_paint.h"
#include "frame_border.h"
#include "lib-win32-style/application/menu/row_text.h"
#include "lib-win32-style/application/graphics/font_pair.h"
#include "lib-win32-style/application/window/caption_button.h"
typedef struct style_client_frame_paint_state {
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
    const style_menu_metrics *menu_metrics;
    BOOL close_disabled;
    HICON icon;
} style_client_frame_paint_state;
typedef void (*style_client_frame_state_read)(void *,style_client_frame_paint_state *);
void style_client_frame_caption(HDC dc,const style_full_client_layout *layout,const style_window_palette *palette,
    const style_client_frame_paint_state *state,PCWSTR title,BOOL child,BOOL zoomed);
#endif
