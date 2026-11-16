#ifndef WIN2K_WINDOW_SYSTEM_MENU_H
#define WIN2K_WINDOW_SYSTEM_MENU_H
#include <windows.h>

/* Paint native system-menu rows; leaves application-owned menus untouched. */
BOOL win2k_system_menu_paint(HWND popup,HMENU menu,HDC dc,BOOL window_coordinates);
void win2k_system_menu_begin(HWND owner);
void win2k_system_menu_set(HWND owner,HMENU menu);
void win2k_system_menu_idle(HWND owner);
void win2k_system_menu_end(HWND owner,BOOL all);
#endif
