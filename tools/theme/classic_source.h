#ifndef CLASSIC_THEME_SOURCE_H
#define CLASSIC_THEME_SOURCE_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

typedef struct classic_caption_source {
    uint32_t start, end, text;
} classic_caption_source;

typedef struct classic_theme_source {
    const char *stem, *label;
    classic_caption_source active, inactive;
    uint32_t face, edge, bevel_light;
    bool gradient, legacy_caption_images, own_sizebox;
    classic_button_group buttons;
    classic_caption_metrics caption_metrics;
    classic_scroll_metrics scroll_metrics;
    const char *fonts[3]; /* caption, small caption, menu */
    classic_menu_colors menu;
    classic_menu_metrics menu_metrics;
    COLORREF choice_face,choice_pressed_face,choice_ink,choice_muted_ink;
    COLORREF group_disabled_text;
    COLORREF selection,selection_text;
    COLORREF selected_tab;
    COLORREF progress_trough;
    bool own_progress_states;
    classic_dialog_metrics dialog_metrics;
    classic_dialog_policy dialog_policy;
    unsigned host_options;
    style_caption_presentation caption_presentation;
    style_border_presentation border_presentation;
    classic_control_presentation controls;
    style_scroll_arrow_source arrow_source;
    COLORREF pressed_toolbar;
} classic_theme_source;

static const classic_theme_source classic_sources[]={
    {
        "Win16","Windows 3.x",
        {RGB(164,200,240),RGB(164,200,240),RGB(0,0,0)},
        {RGB(255,255,255),RGB(255,255,255),RGB(0,0,0)},
        RGB(192,192,192),RGB(0,0,0),RGB(255,255,255),false,true,false,
        WIN16_TITLE_BUTTON_GROUP,
        {18,0,{19,18},{0,0}},
        {CLASSIC_SCROLL_EXTENT_CROSS_AXIS,0,STYLE_SCROLL_THUMB_FIXED},
        {"MS Sans Serif, 10, Bold, Quality:NonAntialiased",NULL,"System, 12, Bold, Quality:NonAntialiased"},
        {RGB(255,255,255),RGB(164,200,240),RGB(0,0,0),RGB(0,0,0),RGB(0,0,0),RGB(0,0,0),TRUE},
        WIN16_POPUP_MENU_METRICS,
        RGB(255,255,255),RGB(255,255,255),RGB(0,0,0),RGB(0,0,0),
        RGB(0,0,0),
        RGB(164,200,240),RGB(0,0,0),
        RGB(255,255,255),
        RGB(255,255,255),true,
        {.file={0}},
        {CLASSIC_DIALOG_WIN16,CLASSIC_DIALOG_WIN16,CLASSIC_DIALOG_WIN16,CLASSIC_DIALOG_WIN16},
        MSSTYLES_HOST_ALL,
        {STYLE_CAPTION_TEXT_CENTER_CLIP,STYLE_CAPTION_SEPARATOR_AFTER,STYLE_CAPTION_BUTTON_INDEXED,
            STYLE_CAPTION_BUTTON_EDGE_OUTLINE},
        WIN16_FRAME_BORDER,{.separator=STYLE_LINE_FLAT,.group=STYLE_LINE_FLAT,.edit=STYLE_LINE_FLAT,.push=STYLE_PUSH_STEPPED,
            .progress=STYLE_PROGRESS_OUTLINED,.trackbar=STYLE_TRACKBAR_OUTLINED,.header=STYLE_LINE_FLAT,.tab=STYLE_LINE_FLAT},
        STYLE_SCROLL_ARROW_INDEXED,RGB(128,128,128)
    },
    {
        "Win2K","Windows 2000",
        {RGB(0,0,128),RGB(166,202,240),RGB(255,255,255)},
        {RGB(128,128,128),RGB(181,181,181),RGB(192,192,192)},
        RGB(212,208,200),RGB(64,64,64),RGB(212,208,200),true,false,true,
        WIN2000_TITLE_BUTTON_GROUP,
        {19,16,{16,14},{15,15}},
        {CLASSIC_SCROLL_EXTENT_SYSTEM,0,STYLE_SCROLL_THUMB_PAGE},
        {"Tahoma, 8, Bold","Tahoma, 6, Bold","Tahoma, 8"},
        {RGB(212,208,200),RGB(0,0,128),RGB(0,0,0),RGB(255,255,255),RGB(128,128,128),RGB(128,128,128),TRUE},
        WIN2000_POPUP_MENU_METRICS,
        RGB(255,255,255),RGB(212,208,200),RGB(0,0,0),RGB(128,128,128),
        RGB(128,128,128),
        RGB(10,36,106),RGB(255,255,255),
        RGB(212,208,200),
        RGB(212,208,200),false,
        WIN2000_DIALOG_METRICS,
        {CLASSIC_DIALOG_WIN2K,CLASSIC_DIALOG_WIN2K,CLASSIC_DIALOG_WIN2K,CLASSIC_DIALOG_WIN2K},
        0,
        {STYLE_CAPTION_TEXT_LEFT_ELLIPSIS,STYLE_CAPTION_SEPARATOR_INSIDE,STYLE_CAPTION_BUTTON_SYSTEM,
            STYLE_CAPTION_BUTTON_NO_OUTLINE},
        WIN2000_FRAME_BORDER,{.separator=STYLE_LINE_RELIEF,.group=STYLE_LINE_RELIEF,.edit=STYLE_LINE_RELIEF,.push=STYLE_PUSH_RELIEF,
            .progress=STYLE_PROGRESS_INSET,.trackbar=STYLE_TRACKBAR_RELIEF,.header=STYLE_LINE_RELIEF,.tab=STYLE_LINE_RELIEF},
        STYLE_SCROLL_ARROW_SYMBOL,RGB(212,208,200)
    }
};

#endif
