#ifndef STYLE_MENU_TRACKING_H
#define STYLE_MENU_TRACKING_H
#include <windows.h>

enum { STYLE_MENU_NO_ITEM=-3, STYLE_MENU_SYSTEM_ITEM=-2 };
typedef enum style_menu_navigation {
    STYLE_MENU_TRACK_COLUMNS, STYLE_MENU_TRACK_NATIVE_POPUPS
} style_menu_navigation;
typedef struct style_menu_tracking {
    HWND window;
    HMENU root,active;
    RECT button;
    int selected,target,direction;
    BOOL armed,escape,double_click;
    LPARAM point;
    void *context;
    int (*hit)(const struct style_menu_tracking *,POINT);
    style_menu_navigation navigation;
    /* Runtime-owned while linked; the caller keeps this object and its
     * callback module alive until end returns. No private renderer state. */
    struct style_menu_tracking *previous;
    void (*filter)(struct style_menu_tracking *,MSG *);
    int kind;
} style_menu_tracking;

/* Same GUI thread only. One shared runtime stack and one observer per actual
 * message source. Supported kinds are WH_GETMESSAGE and WH_MSGFILTER; failed
 * installation publishes no frame. End supports non-top retirement. */
BOOL style_menu_tracking_begin(style_menu_tracking *,int,void (*)(style_menu_tracking *,MSG *));
BOOL style_menu_tracking_end(style_menu_tracking *);
style_menu_tracking *style_menu_tracking_current(void);
void style_menu_tracking_update(HWND,UINT,WPARAM,LPARAM);
/* Stateless input execution, linked by GUI callers rather than the console
 * runtime. Geometry is supplied by hit; native arrow and mouse semantics are
 * selected by navigation. These operations do not install hooks. */
void style_menu_tracking_input(style_menu_tracking *,MSG *);
#endif
