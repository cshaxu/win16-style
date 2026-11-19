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
} classic_theme_source;

static const classic_theme_source classic_sources[]={
    {
        "Win16","Windows 3.x",
        {RGB(164,200,240),RGB(164,200,240),RGB(0,0,0)},
        {RGB(255,255,255),RGB(255,255,255),RGB(0,0,0)},
        RGB(192,192,192),RGB(0,0,0),RGB(255,255,255),false,true,false,
        WIN16_TITLE_BUTTON_GROUP,
        {18,0,{19,18},{0,0}},
        {CLASSIC_SCROLL_EXTENT_CROSS_AXIS,0},
        {"MS Sans Serif, 10, Bold, Quality:NonAntialiased",NULL,"System, 12, Bold, Quality:NonAntialiased"},
        {RGB(255,255,255),RGB(164,200,240),RGB(0,0,0),RGB(0,0,0),RGB(0,0,0),RGB(0,0,0),TRUE},
        WIN16_POPUP_MENU_METRICS,
        RGB(255,255,255),RGB(255,255,255),RGB(0,0,0),RGB(0,0,0),
        RGB(0,0,0),
        RGB(164,200,240),RGB(0,0,0),
        RGB(255,255,255),
        RGB(255,255,255),true,
        {.file={0}}
    },
    {
        "Win2K","Windows 2000",
        {RGB(0,0,128),RGB(166,202,240),RGB(255,255,255)},
        {RGB(128,128,128),RGB(181,181,181),RGB(192,192,192)},
        RGB(212,208,200),RGB(64,64,64),RGB(212,208,200),true,false,true,
        WIN2000_TITLE_BUTTON_GROUP,
        {19,16,{16,14},{15,15}},
        {CLASSIC_SCROLL_EXTENT_SYSTEM,0},
        {"Tahoma, 8, Bold","Tahoma, 6, Bold","Tahoma, 8"},
        {RGB(212,208,200),RGB(0,0,128),RGB(0,0,0),RGB(255,255,255),RGB(128,128,128),RGB(128,128,128),TRUE},
        {0},
        RGB(255,255,255),RGB(212,208,200),RGB(0,0,0),RGB(128,128,128),
        RGB(128,128,128),
        RGB(10,36,106),RGB(255,255,255),
        RGB(212,208,200),
        RGB(212,208,200),false,
        WIN2000_DIALOG_METRICS
    }
};

#endif
