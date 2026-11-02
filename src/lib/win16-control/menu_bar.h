#ifndef WIN16_STYLE_MENU_BAR_H
#define WIN16_STYLE_MENU_BAR_H
#include "menu_paint.h"
#include "menu_layout.h"
typedef struct win16_style_bar_item {
    WCHAR *text;
    UINT state,id;
    HMENU popup;
    HBITMAP control;
    UINT position;
    BOOL owner_draw;
    ULONG_PTR data;
} win16_style_bar_item;
typedef struct win16_style_bar {
    HWND owner;
    HMENU menu;
    UINT count;
    int height;
    RECT *rects;
    win16_style_bar_item *items;
} win16_style_bar;
/* Transient copied text/geometry; HMENU and application itemData are borrowed.
 * The menu-only reader handles strings/standard merged controls. */
BOOL win16_style_bar_read(HMENU menu,HFONT font,UINT dpi,POINT origin,int width,win16_style_bar *bar);
/* Application-owned cells retain their native measurement/drawing callbacks. */
BOOL win16_style_bar_read_for_window(HWND owner,HMENU menu,HFONT font,UINT dpi,POINT origin,int width,win16_style_bar *bar);
void win16_style_bar_free(win16_style_bar *bar);
BOOL win16_style_bar_item_current(const win16_style_bar *bar,UINT item);
void win16_style_bar_paint(const win16_style_bar *bar,HDC dc,HFONT font,UINT dpi,const win16_style_palette *palette);
/* Internal shared original-resource painter for MDI's system/restore cells. */
void win16_style_merged_control(HDC dc,RECT bounds,BOOL system,BOOL selected,int unit,const win16_style_palette *palette);
#endif
