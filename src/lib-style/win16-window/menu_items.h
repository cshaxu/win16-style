#ifndef WIN16_STYLE_MENU_ITEMS_H
#define WIN16_STYLE_MENU_ITEMS_H
#include "menu_paint.h"
#include "lib-style/common/menu_items.h"
BOOL win16_style_menu_draw(style_menu_item *items,const DRAWITEMSTRUCT *draw,
    HFONT font,UINT dpi,const win16_style_palette *palette);
BOOL win16_style_menu_size(style_menu_item *items,MEASUREITEMSTRUCT *measure,HFONT font,UINT dpi);
#endif
