#ifndef STYLE_MSSTYLES_CLASSIC_THEME_H
#define STYLE_MSSTYLES_CLASSIC_THEME_H

#include "classic_style.h"
#include <windows.h>

typedef struct classic_caption_colors {
    COLORREF start, end, text;
} classic_caption_colors;

typedef struct classic_surface_colors {
    COLORREF face, edge;
} classic_surface_colors;

typedef struct classic_theme {
    classic_caption_colors active, inactive;
    classic_surface_colors button, scroll_arrow;
    classic_button_group title_buttons;
    BOOL gradient;
} classic_theme;

/* Reads the selected variant of a v4 classic theme as data only. All output
 * values are owned by the caller and the output is unchanged on failure. */
HRESULT classic_theme_load(PCWSTR path,PCWSTR size,PCWSTR color,classic_theme *theme);

#endif
