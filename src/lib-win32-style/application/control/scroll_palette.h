#ifndef STYLE_SCROLL_PALETTE_H
#define STYLE_SCROLL_PALETTE_H
#include <windows.h>

typedef struct style_scroll_surface_colors {
    COLORREF face, light, highlight, shadow, dark;
} style_scroll_surface_colors;

typedef struct style_native_scroll_colors {
    style_scroll_surface_colors thumb[2], track[2]; /* horizontal, vertical */
    style_scroll_surface_colors sizebox;
    BOOL tracks_and_thumbs;
    BOOL own_sizebox;
} style_native_scroll_colors;
#endif
