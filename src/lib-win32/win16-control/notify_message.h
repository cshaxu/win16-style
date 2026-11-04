#ifndef WIN16_CONTROL_NOTIFY_MESSAGE_H
#define WIN16_CONTROL_NOTIFY_MESSAGE_H
#include "control_state.h"
#include <commctrl.h>
typedef struct track_paint_scope {
    struct track_paint_scope *previous;
    control_style *owner;
    const NMCUSTOMDRAW *draw;
    HDC dc;
    UINT_PTR item;
    RECT rect;
    BOOL active,pending;
} track_paint_scope;
LRESULT win16_control_trackbar_notify(HWND parent,UINT message,WPARAM wparam,LPARAM lparam,UINT_PTR id,control_style *state,SUBCLASSPROC callback,track_paint_scope **active_scope);
LRESULT win16_control_toolbar_notify(HWND parent,UINT message,WPARAM wparam,LPARAM lparam,UINT_PTR id,control_style *state,SUBCLASSPROC callback);
#endif
