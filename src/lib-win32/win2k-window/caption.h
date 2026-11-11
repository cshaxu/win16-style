#ifndef WIN2K_WINDOW_CAPTION_H
#define WIN2K_WINDOW_CAPTION_H
#include <windows.h>

typedef struct win2k_caption_colors {
    COLORREF left, right, text;
    BOOL gradient;
} win2k_caption_colors;

/* The caller owns the DC and font. The rectangle is in DC coordinates. */
BOOL win2k_caption_paint(HDC dc,const RECT *bounds,PCWSTR title,HFONT font,
                         const win2k_caption_colors *colors);
#endif
