#ifndef WIN16_STYLE_MENU_BAR_H
#define WIN16_STYLE_MENU_BAR_H
#include "menu_paint.h"
#include "../win16-common/menu_layout.h"
#include "../win16-common/layout.h"
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
int win16_control_menu_bar_mnemonic(const win16_style_bar *bar,WCHAR key);
void win16_style_bar_paint(const win16_style_bar *bar,HDC dc,HFONT font,UINT dpi,const win16_style_palette *palette);
/* Internal shared original-resource painter for MDI's system/restore cells. */
void win16_style_merged_control(HDC dc,RECT bounds,BOOL system,BOOL selected,int unit,const win16_style_palette *palette);
/* Whether native menu navigation has reached the edge of the active popup. */
int win16_control_menu_bar_hit_screen(HWND window,const win16_style_layout *layout,
    HFONT font,UINT dpi,POINT point);
int win16_control_menu_bar_item_screen(HWND window,const win16_style_layout *layout,
    HFONT font,UINT dpi,LPARAM point,BOOL *system_close);
int win16_control_menu_bar_next(int selected,int direction,int count,BOOL system);
typedef struct win16_control_bar_state_view {
    void *context;
    BOOL *detached,*keyboard;
    HWND *focus;
    int *selected;
    HFONT *font;
    UINT *dpi;
    BOOL (*layout)(HWND,void *,win16_style_layout *);
    void (*paint)(HWND,void *);
} win16_control_bar_state_view;
void win16_control_menu_bar_leave(HWND window,const win16_control_bar_state_view *state,BOOL restore_focus);
void win16_control_menu_bar_select(HWND window,const win16_control_bar_state_view *state,int direction);
typedef struct win16_control_menu_input_view {
    HWND *window;
    HMENU *root,*active;
    RECT *button;
    void *hit_context;
    int (*hit)(void *,POINT);
    int *selected,*target,*direction;
    BOOL *escape,*double_click;
    LPARAM *point;
} win16_control_menu_input_view;
void win16_control_menu_input(MSG *message,const win16_control_menu_input_view *view);
#endif
