#ifndef STYLE_MENU_OWNER_STATE_H
#define STYLE_MENU_OWNER_STATE_H
#include "menu_items.h"
#include "popup.h"

typedef struct style_menu_loop style_menu_loop;
typedef struct style_menu_bar_state {
    int selected,highlighted,direction,hover;
    BOOL keyboard;
    HWND focus;
} style_menu_bar_state;
typedef struct style_menu_owner {
    style_menu_item *items;
    style_menu_loop *loops;
    unsigned depth;
    BOOL system,detached;
    style_menu_bar_state bar;
} style_menu_owner;
typedef struct style_menu_owner_ops {
    style_menu_popup_scope *(*begin)(void *,HWND);
    void (*system)(void *,style_menu_popup_scope *,HMENU);
    BOOL (*measure)(void *,style_menu_item *,MEASUREITEMSTRUCT *);
    BOOL (*draw)(void *,style_menu_item *,const DRAWITEMSTRUCT *);
    void (*selection)(void *,HWND,BOOL);
    BOOL adapt_items;
} style_menu_owner_ops;

/* Zero-initialized, GUI-thread-owned state. The caller pins its enclosing
 * window/host through synchronous native and renderer callbacks and stops
 * this owner before releasing fonts or the enclosing object. A failed entry
 * still counts native depth, so its exit cannot pop an outer successful scope.
 * Native HMENU/itemData/font handles remain borrowed. */
BOOL style_menu_owner_enter(style_menu_owner *,HWND,const style_menu_owner_ops *,void *);
void style_menu_owner_exit(style_menu_owner *);
void style_menu_owner_stop(style_menu_owner *);
BOOL style_menu_bar_select(style_menu_owner *,HWND,int);
void style_menu_bar_leave(style_menu_owner *,HWND,BOOL);
int style_menu_bar_step(int,int,int,BOOL);
style_menu_popup_scope *style_menu_owner_scope(const style_menu_owner *);
/* TRUE means native forwarding has already occurred or the message was
 * consumed. FALSE requires the caller's ordinary native forwarding path.
 * Adapted items initialize after forwarding; native items retain the existing
 * pre-forward system-menu setup. No hooks or propagation are installed here. */
BOOL style_menu_owner_message(style_menu_owner *,HWND,UINT,WPARAM,LPARAM,
    const style_menu_owner_ops *,void *,LRESULT *);
#endif
