#ifndef STYLE_CONTROL_NATIVE_H
#define STYLE_CONTROL_NATIVE_H
#include "basic.h"
/* Actual COMCTL protocol families, independent of a rendering profile. */
typedef enum style_control_native_kind {
    STYLE_CONTROL_TAB,STYLE_CONTROL_HEADER,STYLE_CONTROL_PROGRESS
} style_control_native_kind;
typedef struct style_control_native_state {
    unsigned epoch;
    int pressed;
} style_control_native_state;
void style_control_native_init(style_control_native_state *);
BOOL style_control_native_changes(style_control_native_kind,UINT);
void style_control_native_observe(style_control_native_state *,style_control_native_kind,UINT);
/* Caller pins the whole instance during synchronous queries/render callbacks.
 * Support is profile policy; draw handles buffering versus borrowed print DC.
 * Observe runs at message entry, before native forwarding or item snapshots. */
LRESULT style_control_native_message(HWND,style_instance *,style_control_native_state *,style_control_native_kind,
    UINT,WPARAM,LPARAM,BOOL (*supported)(HWND),const style_control_draw *);
static inline BOOL style_control_native_current(HWND window,const style_instance *instance,const style_control_native_state *state,unsigned epoch)
{return !instance->detached && state->epoch==epoch && IsWindow(window);}
#endif
