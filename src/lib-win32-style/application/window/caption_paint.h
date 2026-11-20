#ifndef STYLE_WINDOW_CAPTION_PAINT_H
#define STYLE_WINDOW_CAPTION_PAINT_H

#include <stdint.h>
#include <windows.h>
#include "caption_slots.h"

enum style_caption_text_mode {
    STYLE_CAPTION_TEXT_CENTER_CLIP=1,
    STYLE_CAPTION_TEXT_LEFT_ELLIPSIS
};
enum style_caption_separator {
    STYLE_CAPTION_SEPARATOR_NONE,
    STYLE_CAPTION_SEPARATOR_INSIDE,
    STYLE_CAPTION_SEPARATOR_AFTER
};
enum style_caption_button_source {
    STYLE_CAPTION_BUTTON_DEFAULT, STYLE_CAPTION_BUTTON_INDEXED, STYLE_CAPTION_BUTTON_SYSTEM
};
enum style_caption_button_outline {
    STYLE_CAPTION_BUTTON_NO_OUTLINE, STYLE_CAPTION_BUTTON_EDGE_OUTLINE
};
typedef struct style_caption_presentation {
    uint8_t text_mode,separator,button_source,button_outline;
} style_caption_presentation;

/* Receives DC coordinates. fill_right ends the gradient; remaining pixels
 * before the band's right edge use the end color. Returns the gradient band
 * in which the caller can lay out text. */
RECT style_caption_band(HDC dc,RECT band,int fill_right,COLORREF start,COLORREF end,
    BOOL gradient,unsigned separator,int unit,COLORREF line);
BOOL style_caption_title(HDC dc,RECT bounds,PCWSTR title,HFONT font,COLORREF color,unsigned mode);
BOOL style_caption_fill(HDC dc,RECT bounds,COLORREF start,COLORREF end,BOOL gradient);

typedef struct style_caption_raster {
    const BYTE *roles;
    unsigned width,height,stride;
    BYTE first_role;
    const COLORREF *palette_override;
} style_caption_raster;

/* The raster role bytes are borrowed for the draw call. The caller supplies
 * the destination and the four role colors, including any state overrides. */
void style_caption_raster_paint(HDC dc,RECT bounds,const style_caption_raster *raster,
    const COLORREF colors[4]);
/* Colors correspond to face, shadow, dark shadow and light. */
void style_caption_system_paint(HDC dc,RECT bounds,UINT glyph,const COLORREF colors[4]);
/* Returns FALSE if the role has neither an admitted raster nor a system glyph. */
BOOL style_caption_button_paint(HDC dc,RECT bounds,enum classic_button_role role,
    BOOL maximized,UINT flags,int unit,const style_caption_presentation *presentation,
    const style_caption_raster *raster,const COLORREF colors[4]);

#endif
