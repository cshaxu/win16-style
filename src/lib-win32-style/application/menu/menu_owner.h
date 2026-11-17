#ifndef STYLE_MENU_OWNER_H
#define STYLE_MENU_OWNER_H
#include <windows.h>
BOOL style_menu_owner_mark(HWND window,const void *caller,LPCWSTR class_name,LPCWSTR property,HANDLE marker);
BOOL style_menu_owner_eligible(HWND window,LPCWSTR property,HANDLE marker);
/* Stack-owned temporary attachment identity. The caller supplies its real
 * install result and removal policy; no installation is invented here.
 * Clear before removal reentry and remove only a live owned window.
 * Keep this exact address alive through native tracking and scope end. */
typedef struct style_menu_owner_scope {
    HWND window;
    BOOL installed;
    BOOL (*remove)(HWND,const void *);
} style_menu_owner_scope;
void style_menu_owner_scope_end(void *);
#endif
