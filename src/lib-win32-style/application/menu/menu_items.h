#ifndef STYLE_MENU_ITEMS_H
#define STYLE_MENU_ITEMS_H
#include <windows.h>
/* Temporary owner-draw data; restore before the owning menu is released. */
typedef struct style_menu_item {
    struct style_menu_item *next;
    HMENU menu;
    UINT type;
    ULONG_PTR data;
    WCHAR *text;
    HBITMAP bitmap;
    BOOL submenu;
    unsigned depth;
} style_menu_item;
style_menu_item *style_menu_item_find(style_menu_item *,ULONG_PTR);
BOOL style_menu_prepare(HMENU,style_menu_item **,unsigned);
void style_menu_restore_depth(unsigned,style_menu_item **);
void style_menu_restore(style_menu_item **);
void style_menu_restore_popup(HMENU,style_menu_item **);
BOOL style_menu_char(style_menu_item *,HMENU,WCHAR,LRESULT *);
#endif
