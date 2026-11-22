#ifndef STYLE_UWP_CLIENT_FRAME_H
#define STYLE_UWP_CLIENT_FRAME_H
#include "lib-win32-style/application/adapters/frame.h"
#include "lib-win32-style/application/window/client_frame_paint.h"
#include "lib-win32-style/application/menu/menu_art.h"

typedef struct style_uwp_client_theme {
    classic_button_group title_buttons;
    classic_caption_metrics caption_metrics;
    style_caption_presentation caption_presentation;
    style_border_presentation border_presentation;
    style_font_descriptor caption_font,menu_font;
    style_window_palette palette;
    classic_menu_colors menu_bar,menu_popup;
    style_menu_metrics menu_metrics;
    COLORREF active_end,inactive_end;
    BOOL gradient;
    style_menu_art menu_art;
    classic_caption_art art;
    BOOL (*refresh_fonts)(HWND,style_font_pair *,const style_font_descriptor *,const style_font_descriptor *);
} style_uwp_client_theme;

BOOL style_uwp_client_frame_apply(HWND,HWND,const style_uwp_frame_action *,unsigned,const style_uwp_client_theme *);
#endif
