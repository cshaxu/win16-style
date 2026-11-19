#ifndef CLASSIC_THEME_SOURCE_H
#define CLASSIC_THEME_SOURCE_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"

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
} classic_theme_source;

static const classic_theme_source classic_sources[]={
    {
        "Win16","Windows 3.x",
        {RGB(164,200,240),RGB(164,200,240),RGB(0,0,0)},
        {RGB(255,255,255),RGB(255,255,255),RGB(0,0,0)},
        RGB(192,192,192),RGB(0,0,0),RGB(255,255,255),false,true,false,
        WIN16_TITLE_BUTTON_GROUP,
        {18,0,{19,18},{0,0}},
        {CLASSIC_SCROLL_EXTENT_CROSS_AXIS,0}
    },
    {
        "Win2K","Windows 2000",
        {RGB(0,0,128),RGB(166,202,240),RGB(255,255,255)},
        {RGB(128,128,128),RGB(181,181,181),RGB(192,192,192)},
        RGB(212,208,200),RGB(64,64,64),RGB(212,208,200),true,false,true,
        WIN2000_TITLE_BUTTON_GROUP,
        {19,16,{16,14},{15,15}},
        {CLASSIC_SCROLL_EXTENT_SYSTEM,0}
    }
};

#endif
