#ifndef WIN16_CONTROL_FRAME_STATE_H
#define WIN16_CONTROL_FRAME_STATE_H
#include "../common/palette.h"
#include "lib-win32-style/application/menu/menu_art.h"
#include "lib-win32-style/application/graphics/font_pair.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include "lib-win32-style/application/window/native_frame.h"
#include "lib-win32-style/application/window/caption_button.h"
#include "lib-win32-style/application/window/caption_slots.h"
#include "lib-win32-style/application/menu/client_popup.h"
#include "lib-win32-style/application/menu/owned_rows.h"
#include "lib-win32-style/application/adapters/repaint.h"
typedef struct style_window {
    style_instance instance;
    win16_style_palette palette;
    classic_button_group title_buttons;
    classic_caption_metrics caption_metrics;
    style_caption_presentation caption_presentation;
    style_border_presentation border_presentation;
    BOOL caption_gradient;
    COLORREF active_end,inactive_end;
    msstyles_font caption_font,menu_font;
    classic_menu_colors menu_bar, menu_popup;
    classic_menu_metrics menu_metrics;
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
} style_window;

style_window *win16_control_frame_state_create(void);
void win16_control_frame_state_dispose(void *context);
#endif
