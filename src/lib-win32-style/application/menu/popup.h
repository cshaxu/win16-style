#ifndef STYLE_MENU_POPUP_H
#define STYLE_MENU_POPUP_H
#include <windows.h>

typedef struct style_menu_popup style_menu_popup;
typedef struct style_menu_popup_scope style_menu_popup_scope;
typedef struct style_menu_popup_ops {
    BOOL (*accept)(void *,HWND);
    void (*border)(style_menu_popup *,HDC);
    BOOL (*paint)(style_menu_popup *,HDC,BOOL);
    BOOL (*selection)(style_menu_popup *,ULONGLONG *);
    void (*dispose)(void *);
    BOOL suppress_shadow,keyboard_refresh;
} style_menu_popup_ops;

/* GUI-thread scopes share one observer, including scopes from different
 * renderers. ops must outlive the scope; context transfers only on success.
 * End may remove a non-top scope. Context disposal waits for active popup
 * callbacks, so ending a scope from paint/native dispatch is supported.
 * Module lifetime remains the caller's responsibility. No desktop hook. */
style_menu_popup_scope *style_menu_popup_begin(HWND,const style_menu_popup_ops *,void *);
BOOL style_menu_popup_end(style_menu_popup_scope *);
void *style_menu_popup_scope_context(style_menu_popup_scope *);
void style_menu_popup_idle(style_menu_popup_scope *);
HWND style_menu_popup_window(const style_menu_popup *);
void *style_menu_popup_context(const style_menu_popup *);
BOOL style_menu_popup_live(const style_menu_popup *);
#endif
