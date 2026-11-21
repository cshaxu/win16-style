#ifndef STYLE_CLIENT_FRAME_STATE_H
#define STYLE_CLIENT_FRAME_STATE_H
#include "palette.h"
#include "caption_paint.h"
#include "frame_border.h"
#include "lib-win32-style/application/menu/row_text.h"
#include "lib-win32-style/application/menu/menu_art.h"
#include "lib-win32-style/application/graphics/font_pair.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include "lib-win32-style/application/window/native_frame.h"
#include "lib-win32-style/application/window/caption_button.h"
#include "lib-win32-style/application/window/caption_slots.h"
#include "lib-win32-style/application/menu/owner.h"
#include "lib-win32-style/application/adapters/repaint.h"
typedef struct style_client_frame {
    style_instance instance;
    style_window_palette palette;
    classic_button_group title_buttons;
    classic_caption_metrics caption_metrics;
    style_caption_presentation caption_presentation;
    style_border_presentation border_presentation;
    BOOL caption_gradient;
    COLORREF active_end,inactive_end;
    style_font_descriptor caption_font,menu_font;
    classic_menu_colors menu_bar, menu_popup;
    style_menu_metrics menu_metrics;
    style_menu_art menu_art;
    classic_caption_art art;
    style_font_pair fonts;
    BOOL dialog, menu_only;
    style_menu_owner menu;
    style_native_frame native;
    style_caption_button button;
    BOOL xaml_icon_update;
    style_uwp_repaint xaml;
    void (*xaml_activate)(HWND,void *,const RECT *,BOOL);
} style_client_frame;

style_client_frame *style_client_frame_state_create(void);
void style_client_frame_state_dispose(void *context);
#endif
