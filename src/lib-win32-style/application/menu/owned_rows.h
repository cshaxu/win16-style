#ifndef STYLE_MENU_OWNED_ROWS_H
#define STYLE_MENU_OWNED_ROWS_H
#include "row_paint.h"
#include "lib-win32-style/application/menu/owner.h"
BOOL style_menu_owned_draw(style_menu_item *items,const DRAWITEMSTRUCT *draw,
    HFONT font,UINT dpi,const style_window_palette *palette,const classic_menu_colors *colors,
    const style_menu_metrics *metrics,const style_menu_art *art);
BOOL style_menu_owned_measure(style_menu_item *items,MEASUREITEMSTRUCT *measure,HFONT font,UINT dpi,
    const style_menu_metrics *metrics);
typedef struct style_menu_client_theme {
    const style_window_palette *palette;
    HFONT font;
    UINT dpi;
    void *context;
    void (*selection)(HWND,void *,BOOL);
    const classic_menu_colors *colors;
    const style_menu_metrics *metrics;
    const style_menu_art *art;
    const style_menu_owner *owner;
} style_menu_client_theme;
extern const style_menu_owner_ops style_menu_client_operations;
#endif
