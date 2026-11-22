#ifndef STYLE_OBSERVED_FRAME_H
#define STYLE_OBSERVED_FRAME_H
#include "native_caption.h"
#include "lib-win32-style/application/control/native_scroll.h"
#include "lib-win32-style/application/menu/native_system_menu.h"

typedef struct style_observed_frame_theme {
    BOOL present;
    classic_button_group title_buttons;
    classic_caption_metrics caption_metrics;
    style_caption_presentation caption_presentation;
    style_border_presentation border_presentation;
    style_frame_colors colors;
    style_native_caption_colors active,inactive;
    style_font_descriptor caption_font,small_caption_font,menu_font;
    classic_menu_colors menu_bar;
    unsigned menu_text_flags;
    style_native_menu menu;
    classic_caption_art art;
    style_frame_sprites sprites;
    HMODULE sprite_resources; /* Owns the bitmap views when present. */
} style_observed_frame_theme;

typedef struct style_observed_frame_provider {
    /* These names and callbacks must outlive the window's attachment. */
    PCWSTR attachment_property,recalculate_message;
    DWORD (*load)(style_observed_frame_theme *,style_native_scroll *);
    HFONT (*caption_font)(const style_font_descriptor *,UINT dpi,BOOL tool);
} style_observed_frame_provider;

BOOL style_observed_frame_attach(HWND window,const style_observed_frame_provider *);
BOOL style_observed_frame_attached(HWND window);
BOOL style_observed_frame_remove(HWND window);
#endif
