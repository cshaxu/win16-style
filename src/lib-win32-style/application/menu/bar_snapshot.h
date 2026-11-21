#ifndef STYLE_MENU_BAR_SNAPSHOT_H
#define STYLE_MENU_BAR_SNAPSHOT_H
#include <windows.h>

typedef struct style_menu_bar_item {
    WCHAR *text;
    UINT state,id,type,position;
    HMENU popup;
    HBITMAP control;
    BOOL owner_draw;
    ULONG_PTR data;
} style_menu_bar_item;
typedef struct style_menu_bar_snapshot {
    HWND owner;
    HMENU menu;
    UINT count;
    int height;
    RECT *rects;
    style_menu_bar_item *items;
} style_menu_bar_snapshot;
typedef enum style_menu_bar_inclusion {
    STYLE_MENU_BAR_UNSUPPORTED=-1,STYLE_MENU_BAR_SKIP,STYLE_MENU_BAR_INCLUDE
} style_menu_bar_inclusion;
typedef struct style_menu_bar_profile {
    style_menu_bar_inclusion (*include)(void *,const MENUITEMINFOW *,UINT *);
    BOOL (*measure)(void *,HDC,const MENUITEMINFOW *,SIZE *);
    BOOL native_rects;
} style_menu_bar_profile;

/* Owns copied text/items/rectangles, borrows HWND/HMENU/itemData and renderer
 * resources. Caller pins its surrounding instance through synchronous owner
 * callbacks. On failure the snapshot is empty; free is idempotent. Custom
 * geometry uses renderer sizes and classic arrangement; native geometry stays
 * in screen coordinates and ignores origin/width. No menu items are changed. */
BOOL style_menu_bar_read(HWND,HMENU,POINT,int,const style_menu_bar_profile *,void *,style_menu_bar_snapshot *);
void style_menu_bar_free(style_menu_bar_snapshot *);
BOOL style_menu_bar_item_current(const style_menu_bar_snapshot *,UINT);
int style_menu_bar_mnemonic(const style_menu_bar_snapshot *,WCHAR,BOOL);
/* Forward a native application-owned cell with borrowed DC state isolated.
 * FALSE means identity was lost and painting must stop. A failed SaveDC skips
 * that cell without drawing, matching the existing native adapter contract. */
BOOL style_menu_bar_draw_owned(const style_menu_bar_snapshot *,UINT,HDC);
#endif
