#ifndef WIN2K_WINDOW_SYSTEM_MENU_H
#define WIN2K_WINDOW_SYSTEM_MENU_H
#include "lib-win32-style/application/menu/owner.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

typedef struct win2k_menu_style {
    msstyles_font font;
    classic_menu_colors colors;
    classic_menu_metrics metrics;
    style_menu_art art;
} win2k_menu_style;

void win2k_menu_style_read(win2k_menu_style *style,const classic_theme_mapping *source);
/* Paint native system-menu rows; leaves application-owned menus untouched. */
classic_menu_colors win2k_menu_colors(const classic_menu_colors *defined);
BOOL win2k_system_menu_paint(HWND popup,HMENU menu,HDC dc,BOOL window_coordinates,
    const win2k_menu_style *style);
extern const style_menu_owner_ops win2k_system_menu_operations;
#endif
