#ifndef STYLE_MENU_CLIENT_POPUP_H
#define STYLE_MENU_CLIENT_POPUP_H
#include "../window/palette.h"
#include "menu_art.h"
#include "row_text.h"
#include "owner.h"
/* Same-thread lifetime around one native menu loop. Nested scopes are allowed;
 * owner teardown may end an outer scope before an inner scope finishes.
 * Native popup menus shown/painted on that thread are decorated, including
 * windows created before WM_ENTERMENULOOP. Removed at scope exit.
 * The owner is borrowed until it stops the scope. This painter changes
 * pixels only; native menu input and tracking remain with USER. */
style_menu_popup_scope *style_menu_client_popup_begin(const style_window_palette *palette,
    const classic_menu_colors *colors,const style_menu_metrics *metrics,const style_menu_art *art,
    const style_menu_owner *owner,UINT dpi);
void style_menu_client_popup_system(style_menu_popup_scope *,HMENU,HFONT,UINT);
#endif
