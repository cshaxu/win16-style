#ifndef STYLE_CONTROL_BASIC_H
#define STYLE_CONTROL_BASIC_H
#include "control_state.h"

typedef enum style_control_basic_kind {
    STYLE_CONTROL_BUTTON,STYLE_CONTROL_STATIC,STYLE_CONTROL_BITMAP,
    STYLE_CONTROL_SEPARATOR,STYLE_CONTROL_NATIVE_OVERLAY
} style_control_basic_kind;
typedef struct style_control_draw {
    void *context;
    void (*render)(HWND,void *,HDC,BOOL);
} style_control_draw;
/* Caller holds its surrounding instance through native/render callbacks.
 * Buffered is requested for WM_PAINT; print/overlay DCs remain borrowed.
 * Kind selects the existing native control protocol, not a concrete era. */
LRESULT style_control_basic_message(HWND,style_instance *,style_control_state *,UINT,WPARAM,LPARAM,
    style_control_basic_kind,const style_control_draw *);
#endif
