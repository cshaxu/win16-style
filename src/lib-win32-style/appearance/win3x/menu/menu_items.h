#ifndef WIN16_STYLE_MENU_ITEMS_H
#define WIN16_STYLE_MENU_ITEMS_H
#include "menu_paint.h"
#include "lib-win32-style/application/menu/owner.h"
BOOL win16_style_menu_draw(style_menu_item *items,const DRAWITEMSTRUCT *draw,
    HFONT font,UINT dpi,const win16_style_palette *palette);
BOOL win16_style_menu_size(style_menu_item *items,MEASUREITEMSTRUCT *measure,HFONT font,UINT dpi);
typedef struct win16_style_menu_theme {
    const win16_style_palette *palette;
    HFONT font;
    UINT dpi;
    void *context;
    void (*selection)(HWND,void *,BOOL);
} win16_style_menu_theme;
extern const style_menu_owner_ops win16_style_menu_operations;
#endif
