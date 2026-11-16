#ifndef STYLE_CONTROL_RETIRE_H
#define STYLE_CONTROL_RETIRE_H
#include "control_state.h"

typedef struct style_control_retired {
    BOOL result,colors_changed;
} style_control_retired;
typedef struct style_control_retire_ops {
    void (*finish_input)(HWND,void *);
    style_control_retired (*release)(HWND,void *,BOOL);
} style_control_retire_ops;

/* Same GUI thread. Owns reentrant removal, callback refs, subclass detach and
 * native surface refresh. The provider releases its complete live bindings
 * after detach; state storage remains pinned through that operation. Terminal
 * retirement skips gesture completion/restoration and reports child failure.
 * Failed detach retains ownership; neither path unloads a DLL. */
BOOL style_control_retire(HWND,style_instance *,style_control_state *,BOOL,void *,const style_control_retire_ops *);
#endif
