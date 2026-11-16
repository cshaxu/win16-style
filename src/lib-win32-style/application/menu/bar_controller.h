#ifndef STYLE_MENU_BAR_CONTROLLER_H
#define STYLE_MENU_BAR_CONTROLLER_H
#include "bar_snapshot.h"
#include "owner.h"
#include "tracking.h"
#include "../lifecycle/instance.h"

typedef struct style_menu_bar_target style_menu_bar_target;
typedef struct style_menu_bar_ops {
    /* A successful read transfers a temporary snapshot with screen rectangles. */
    BOOL (*read)(const style_menu_bar_target *,style_menu_bar_snapshot *);
    int (*hit)(const style_menu_bar_target *,POINT);
    void (*paint)(const style_menu_bar_target *);
    BOOL (*lookup)(HWND,style_menu_bar_target *);
    style_menu_navigation navigation;
} style_menu_bar_ops;
struct style_menu_bar_target {
    HWND window;
    style_instance *instance;
    style_menu_owner *owner;
    void *context;
    const style_menu_bar_ops *ops;
};

/* Same GUI thread. The controller retains the current instance across native
 * callbacks and pins a NEXTMENU destination before releasing the source.
 * Geometry/resources remain renderer-owned. No new hooks or unload semantics. */
void style_menu_bar_open(style_menu_bar_target,int);
BOOL style_menu_bar_choose(const style_menu_bar_target *,int);
void style_menu_bar_finish(const style_menu_bar_target *,BOOL);
/* Common bar key/mnemonic/pointer protocol. System-caption geometry and
 * caption-button behavior remain the window adapter's responsibility. */
BOOL style_menu_bar_input(const style_menu_bar_target *,UINT,WPARAM,LPARAM);
LRESULT style_menu_bar_system(const style_menu_bar_target *,RECT,BOOL,UINT,WPARAM,LPARAM);
#endif
