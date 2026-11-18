#ifndef STYLE_CONTROL_NOTIFY_H
#define STYLE_CONTROL_NOTIFY_H
#include "../lifecycle/instance.h"
#include <commctrl.h>
typedef struct style_control_notify_state {HWND parent,window;BOOL internal;} style_control_notify_state;
typedef struct style_control_track_scope {
    struct style_control_track_scope *previous;
    style_instance *owner;
    const NMCUSTOMDRAW *draw;
    HDC dc;
    UINT_PTR item;
    RECT rect;
    BOOL active,pending;
} style_control_track_scope;
typedef struct style_control_track_calls {style_control_track_scope *current;} style_control_track_calls;

typedef struct style_control_notify_ops {
    const void *context;
    BOOL (*supported)(HWND);
    BOOL (*part)(HWND,const NMCUSTOMDRAW *,const void *);
    BOOL (*runtime_border)(HWND,const NMCUSTOMDRAW *,const void *);
    void (*prepare_item)(HWND,NMCUSTOMDRAW *,const void *);
    void (*finish_item)(HWND,const NMCUSTOMDRAW *,const void *);
    void (*finish_paint)(HWND,const NMCUSTOMDRAW *,const void *);
} style_control_notify_ops;
/* Caller owns and pins instance/state and provides one GUI-thread-local stack.
 * Bind/unbind preserve the native parent's subclass ownership. No style is
 * selected here; paint operations borrow context for the synchronous call. */
BOOL style_control_notify_bind(HWND,style_control_notify_state *,SUBCLASSPROC,void *);
void style_control_notify_unbind(style_control_notify_state *,SUBCLASSPROC,void *);
LRESULT style_control_notify_message(HWND,UINT,WPARAM,LPARAM,UINT_PTR,style_instance *,style_control_notify_state *,
    SUBCLASSPROC,style_control_track_calls *,const style_control_notify_ops *);
LRESULT style_control_track_dispatch(style_control_track_calls *,style_instance *,HWND,UINT,WPARAM,LPARAM);
/* Caller has selected a trackbar and pins instance/state across forwarding.
 * Parent notification owner is explicit (it need not equal instance). Native
 * style/cache mutation and parent retirement keep their original order. */
LRESULT style_control_track_message(HWND,style_instance *,style_control_notify_state *,UINT,WPARAM,LPARAM,
    SUBCLASSPROC,void *,style_control_track_calls *);
#endif
