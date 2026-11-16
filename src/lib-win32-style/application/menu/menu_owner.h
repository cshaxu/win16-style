#ifndef STYLE_MENU_OWNER_H
#define STYLE_MENU_OWNER_H
#include <windows.h>
BOOL style_menu_owner_mark(HWND window,const void *caller,LPCWSTR class_name,LPCWSTR property,HANDLE marker);
BOOL style_menu_owner_eligible(HWND window,LPCWSTR property,HANDLE marker);
#endif
