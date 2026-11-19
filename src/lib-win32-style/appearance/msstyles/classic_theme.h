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

enum classic_push_state {
    CLASSIC_PUSH_NORMAL, CLASSIC_PUSH_PRESSED, CLASSIC_PUSH_DISABLED,
    CLASSIC_PUSH_DEFAULTED, CLASSIC_PUSH_STATE_COUNT
};

typedef struct classic_push_colors {
    classic_surface_colors state[CLASSIC_PUSH_STATE_COUNT];
} classic_push_colors;

static inline classic_surface_colors classic_push_color(const classic_push_colors *colors,
    BOOL disabled,BOOL pressed,BOOL defaulted)
{
    return colors->state[disabled?CLASSIC_PUSH_DISABLED:pressed?CLASSIC_PUSH_PRESSED:
        defaulted?CLASSIC_PUSH_DEFAULTED:CLASSIC_PUSH_NORMAL];
}

typedef struct classic_scroll_surface {
    COLORREF face, light, highlight, shadow, dark;
} classic_scroll_surface;

typedef struct classic_scroll_colors {
    classic_scroll_surface thumb[2], track[2]; /* horizontal, vertical */
    classic_scroll_surface sizebox;
    BOOL tracks_and_thumbs;
    BOOL own_sizebox;
} classic_scroll_colors;

enum classic_caption_image_role {
    CLASSIC_IMAGE_MINIMIZE, CLASSIC_IMAGE_MAXIMIZE, CLASSIC_IMAGE_RESTORE,
    CLASSIC_IMAGE_SYSTEM, CLASSIC_IMAGE_ROLE_COUNT
};

typedef struct classic_caption_images {
    WORD id[CLASSIC_IMAGE_ROLE_COUNT][2]; /* normal, pressed */
} classic_caption_images;

typedef struct classic_theme {
    classic_caption_colors active, inactive;
    classic_push_colors push;
    classic_surface_colors scroll_arrow;
    classic_button_group title_buttons;
    classic_caption_metrics caption_metrics;
    classic_caption_images caption_images;
    classic_scroll_colors scroll;
    classic_scroll_metrics scroll_metrics;
    BOOL gradient;
} classic_theme;

int classic_scroll_nominal(classic_scroll_metrics metrics,enum classic_scroll_extent_source fallback,
    const RECT *bar,BOOL vertical,UINT dpi);
/* Reads the selected variant from an already mapped resource module. */
HRESULT classic_theme_read(HMODULE module,PCWSTR size,PCWSTR color,classic_theme *theme);
BOOL classic_theme_default(HMODULE module,classic_theme *theme);

/* Reads the selected variant of a v4 classic theme as data only. All output
 * values are owned by the caller and the output is unchanged on failure. */
HRESULT classic_theme_load(PCWSTR path,PCWSTR size,PCWSTR color,classic_theme *theme);

#endif
