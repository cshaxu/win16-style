#ifndef STYLE_WINDOW_PALETTE_H
#define STYLE_WINDOW_PALETTE_H
#include <windows.h>

typedef struct style_frame_colors {
    COLORREF face,light,shadow,edge;
    BOOL present;
} style_frame_colors;

typedef struct style_window_palette {
    COLORREF active, inactive, active_text, inactive_text;
    COLORREF face, light, shadow, edge;
} style_window_palette;
void style_window_palette_default(style_window_palette *);
void style_window_palette_resolve(const style_window_palette *,style_window_palette *);
BOOL style_window_high_contrast(void);
LRESULT style_window_dialog_colors(HDC,const style_window_palette *);
#endif
