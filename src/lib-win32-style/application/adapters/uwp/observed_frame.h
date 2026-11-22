#ifndef STYLE_UWP_OBSERVED_FRAME_H
#define STYLE_UWP_OBSERVED_FRAME_H
#include "lib-win32-style/application/adapters/frame.h"
#include "lib-win32-style/application/menu/native_system_menu.h"
#include "lib-win32-style/application/window/native_caption.h"

typedef struct style_uwp_observed_theme {
    BOOL present;
    classic_button_group buttons;
    classic_caption_metrics metrics;
    style_caption_presentation caption;
    style_border_presentation border;
    style_frame_colors colors;
    style_native_caption_colors active,inactive;
    style_font_descriptor font;
    classic_caption_art art;
    style_native_menu menu;
    HFONT (*create_font)(const style_font_descriptor *,UINT);
    /* A host without theme data may supply its own native defaults. */
    BOOL (*default_buttons)(const RECT *,UINT,BOOL,const BOOL *,RECT *);
    int default_grip_width;
} style_uwp_observed_theme;

BOOL style_uwp_observed_frame_apply(HWND,HWND,const style_uwp_frame_action *,unsigned,const style_uwp_observed_theme *);
#endif
