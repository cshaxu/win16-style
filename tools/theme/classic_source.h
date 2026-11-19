#ifndef CLASSIC_THEME_SOURCE_H
#define CLASSIC_THEME_SOURCE_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct classic_caption_source {
    uint32_t start, end, text;
} classic_caption_source;

typedef struct classic_theme_source {
    const char *stem, *label;
    classic_caption_source active, inactive;
    uint32_t face, edge;
    bool gradient;
} classic_theme_source;

static const classic_theme_source classic_sources[]={
    {
        "Win16","Windows 3.x",
        {RGB(164,200,240),RGB(164,200,240),RGB(0,0,0)},
        {RGB(255,255,255),RGB(255,255,255),RGB(0,0,0)},
        RGB(192,192,192),RGB(0,0,0),false
    },
    {
        "Win2000","Windows 2000",
        {RGB(0,0,128),RGB(166,202,240),RGB(255,255,255)},
        {RGB(128,128,128),RGB(181,181,181),RGB(192,192,192)},
        RGB(212,208,200),RGB(64,64,64),true
    }
};

#endif
