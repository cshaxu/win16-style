#ifndef STYLE_FONT_DESCRIPTOR_H
#define STYLE_FONT_DESCRIPTOR_H
#include <stdbool.h>
#include <windows.h>

enum style_menu_font_mode {
    STYLE_MENU_FONT_DEFAULT,STYLE_MENU_FONT_LOGICAL,STYLE_MENU_FONT_SYSTEM_BITMAP
};
typedef struct style_font_descriptor {
    LOGFONTW logical;
    unsigned points;
    enum style_menu_font_mode menu_mode;
} style_font_descriptor;

/* Realized menu font is a thread-owned lease. The legacy option applies
 * only to a theme that omitted the menu realization record. */
HFONT style_menu_font_acquire(UINT dpi,const style_font_descriptor *defined,bool legacy_bitmap);
void style_menu_font_release(HFONT font);

#endif
