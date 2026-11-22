#ifndef STYLE_WINDOW_CAPTION_PAINT_H
#define STYLE_WINDOW_CAPTION_PAINT_H

#include <stdint.h>
#include <windows.h>
#include "caption_slots.h"
#include "lib-win32-style/application/graphics/dib_strip.h"

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
enum style_caption_icon_mode {
    STYLE_CAPTION_ICON_DEFAULT, STYLE_CAPTION_ICON_HIDDEN, STYLE_CAPTION_ICON_NATIVE
};
typedef struct style_caption_presentation {
    uint8_t text_mode,separator,button_source,button_outline,icon_mode;
    uint8_t text_inset_plus_one; /* Device pixels. Zero selects legacy alignment padding. */
} style_caption_presentation;

/* An explicit system button takes priority over the application icon. */
BOOL style_caption_icon_visible(unsigned mode,BOOL system_button,DWORD style,DWORD extended);
BOOL style_caption_icon_bounds(RECT band,int limit,UINT dpi,RECT *icon);

BOOL style_caption_title(HDC dc,RECT bounds,PCWSTR title,HFONT font,COLORREF color,unsigned mode);
BOOL style_caption_fill(HDC dc,RECT bounds,COLORREF start,COLORREF end,BOOL gradient);

typedef struct style_caption_raster {
    const BYTE *roles;
    unsigned width,height,stride;
} style_caption_raster;

enum classic_caption_image_role {
    CLASSIC_IMAGE_MINIMIZE, CLASSIC_IMAGE_MAXIMIZE, CLASSIC_IMAGE_RESTORE,
    CLASSIC_IMAGE_SYSTEM, CLASSIC_IMAGE_ROLE_COUNT
};

enum { CLASSIC_CAPTION_ART_WIDTH=19, CLASSIC_SYSTEM_ART_WIDTH=18, CLASSIC_CAPTION_ART_HEIGHT=18 };

typedef struct classic_caption_art {
    BYTE roles[CLASSIC_IMAGE_ROLE_COUNT+1][2][CLASSIC_CAPTION_ART_WIDTH*CLASSIC_CAPTION_ART_HEIGHT];
    BOOL available;
} classic_caption_art;

style_caption_raster style_caption_button_raster(const classic_caption_art *art,
    enum classic_button_role role,BOOL maximized,BOOL child,UINT flags);

/* The raster borrows zero-based role bytes. The draw call supplies four role colors. */
void style_caption_raster_paint(HDC dc,RECT bounds,const style_caption_raster *raster,
    const COLORREF colors[4]);
/* Colors correspond to face, shadow, dark shadow and light. */
void style_caption_system_paint(HDC dc,RECT bounds,UINT glyph,const COLORREF colors[4]);
/* Roles without theme art use system glyphs or the system application icon. */
BOOL style_caption_button_paint(HDC dc,RECT bounds,enum classic_button_role role,
    BOOL maximized,UINT flags,int unit,const style_caption_presentation *presentation,
    const style_caption_raster *raster,const COLORREF colors[4]);

typedef struct style_frame_glyph {
    style_dib_strip image;
    SIZE minimum;
    int minimum_dpi;
} style_frame_glyph;

enum style_frame_glyph_selection {
    STYLE_GLYPH_FIXED, STYLE_GLYPH_SIZE, STYLE_GLYPH_DPI
};

typedef struct style_frame_sprite_button {
    style_dib_strip face,glyph;
    style_frame_glyph variants[5];
    RECT content;
    enum style_frame_glyph_selection selection;
} style_frame_sprite_button;

const style_dib_strip *style_caption_glyph(const style_frame_sprite_button *button,SIZE content,UINT dpi);

typedef struct style_frame_sprites {
    style_dib_strip caption,maximized,left,right,bottom;
    style_frame_sprite_button buttons[CLASSIC_BUTTON_CLOSE+1],restore;
} style_frame_sprites;

typedef struct style_caption_render_model {
    RECT band,text,icon_bounds;
    HICON icon;
    SIZE icon_size;
    RECT buttons[CLASSIC_BUTTON_CLOSE+1];
    UINT flags[CLASSIC_BUTTON_CLOSE+1];
    COLORREF start,end,ink;
    COLORREF button_colors[4]; /* edge, light, shadow, face */
    BOOL gradient,maximized,child;
    int unit;
    const style_frame_sprites *sprites;
    UINT dpi;
    BOOL active;
    RECT sprite_band;
} style_caption_render_model;

void style_caption_render(HDC dc,PCWSTR title,HFONT font,const style_caption_render_model *model,
    const style_caption_presentation *presentation,const classic_caption_art *art);

#endif
