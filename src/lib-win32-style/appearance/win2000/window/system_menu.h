#ifndef WIN2K_WINDOW_SYSTEM_MENU_H
#define WIN2K_WINDOW_SYSTEM_MENU_H
#include "lib-win32-style/application/menu/owner.h"

/* Paint native system-menu rows; leaves application-owned menus untouched. */
BOOL win2k_system_menu_paint(HWND popup,HMENU menu,HDC dc,BOOL window_coordinates);
extern const style_menu_owner_ops win2k_system_menu_operations;
#endif
