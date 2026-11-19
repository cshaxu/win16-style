#ifndef CLASSIC_THEME_SOURCE_H
#define CLASSIC_THEME_SOURCE_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "lib-win32-style/appearance/msstyles/classic_style.h"

typedef struct classic_caption_source {
    uint32_t start, end, text;
} classic_caption_source;

typedef struct classic_theme_source {
    const char *stem, *label;
    classic_caption_source active, inactive;
    uint32_t face, edge;
    bool gradient, legacy_minimize;
    classic_button_group buttons;
} classic_theme_source;

static const classic_theme_source classic_sources[]={
    {
        "Win16","Windows 3.x",
        {RGB(164,200,240),RGB(164,200,240),RGB(0,0,0)},
        {RGB(255,255,255),RGB(255,255,255),RGB(0,0,0)},
        RGB(192,192,192),RGB(0,0,0),false,true,
        {3,0,0,{{CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0},
                {CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0},
                {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0}}}
    },
    {
        "Win2000","Windows 2000",
        {RGB(0,0,128),RGB(166,202,240),RGB(255,255,255)},
        {RGB(128,128,128),RGB(181,181,181),RGB(192,192,192)},
        RGB(212,208,200),RGB(64,64,64),true,false,
        {4,0,2,{{CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0},
                {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0},
                {CLASSIC_BUTTON_HELP,CLASSIC_BUTTON_RIGHT,0},
                {CLASSIC_BUTTON_CLOSE,CLASSIC_BUTTON_RIGHT,2}}}
    }
};

#endif
