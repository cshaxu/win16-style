#ifndef STYLE_NATIVE_SYSTEM_MENU_H
#define STYLE_NATIVE_SYSTEM_MENU_H
#include "lib-win32-style/application/menu/owner.h"
#include "lib-win32-style/application/graphics/font_descriptor.h"
#include "row_text.h"
#include "menu_art.h"

typedef struct style_native_menu {
    style_font_descriptor font;
    classic_menu_colors colors;
    style_menu_metrics metrics;
    style_menu_art art;
} style_native_menu;

/* Paint native system-menu rows; leaves application-owned menus untouched. */
classic_menu_colors style_menu_system_colors(const classic_menu_colors *defined);
BOOL style_native_system_menu_paint(HWND popup,HMENU menu,HDC dc,BOOL window_coordinates,
    const style_native_menu *style);
extern const style_menu_owner_ops style_native_system_menu_ops;
#endif
