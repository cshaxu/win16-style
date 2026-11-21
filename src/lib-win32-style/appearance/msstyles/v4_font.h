#ifndef STYLE_MSSTYLES_V4_FONT_H
#define STYLE_MSSTYLES_V4_FONT_H

#include <stdbool.h>
#include <windows.h>

enum style_menu_font_mode {
    STYLE_MENU_FONT_DEFAULT,STYLE_MENU_FONT_LOGICAL,STYLE_MENU_FONT_SYSTEM_BITMAP
};
typedef struct msstyles_font {
    LOGFONTW logical;
    unsigned points;
    enum style_menu_font_mode menu_mode;
} msstyles_font;

/* Reads a referenced RT_STRING into an owned descriptor. On failure the
 * output is unchanged. Font realization remains the caller's responsibility. */
bool msstyles_font_read(HMODULE module,UINT string_id,msstyles_font *font);
static inline LOGFONTW msstyles_font_at_dpi(const msstyles_font *font,UINT dpi)
{
    LOGFONTW logical=font->logical;
    logical.lfHeight=-MulDiv((int)font->points,(int)dpi,72);
    return logical;
}

enum msstyles_font_role {
    MSSTYLES_FONT_CAPTION,
    MSSTYLES_FONT_SMALL_CAPTION,
    MSSTYLES_FONT_MENU
};
/* A declared font takes precedence. Missing declarations use native
 * non-client metrics. The caller owns the returned handle. */
HFONT msstyles_font_create(const msstyles_font *defined,UINT dpi,enum msstyles_font_role role);
/* Realized menu font is a thread-owned lease. The legacy option applies
 * only to a theme that omitted the menu realization record. */
HFONT style_menu_font_acquire(UINT dpi,const msstyles_font *defined,bool legacy_bitmap);
void style_menu_font_release(HFONT font);

#endif
