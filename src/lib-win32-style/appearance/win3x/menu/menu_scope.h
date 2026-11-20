#ifndef WIN16_STYLE_MENU_SCOPE_H
#define WIN16_STYLE_MENU_SCOPE_H
#include "../common/palette.h"
#include "lib-win32-style/application/menu/menu_art.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/menu/popup.h"
typedef style_menu_popup_scope win16_style_menu_scope;
/* Same-thread lifetime around one native menu loop. Nested scopes are allowed;
 * owner teardown may end an outer scope before an inner scope finishes.
 * Native popup menus shown/painted on that thread are decorated, including
 * windows created before WM_ENTERMENULOOP. Removed at scope exit.
 * No menu data, input or native tracking is replaced. */
win16_style_menu_scope *win16_style_menu_begin(const win16_style_palette *palette,
    const classic_menu_colors *colors,const classic_menu_metrics *metrics,const style_menu_art *art);
void win16_style_menu_system(win16_style_menu_scope *,HMENU,HFONT,UINT);
#endif
