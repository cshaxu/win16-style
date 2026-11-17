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
typedef enum style_control_owner_action {
    STYLE_CONTROL_OWNER_NONE,STYLE_CONTROL_OWNER_PAINT,STYLE_CONTROL_OWNER_PRINT,STYLE_CONTROL_OWNER_FRAME
} style_control_owner_action;
typedef struct style_control_owner_ops {
    void *context;
    BOOL (*input)(HWND,void *,UINT,WPARAM,LPARAM);
    style_control_owner_action (*select)(HWND,void *,UINT,LPARAM);
    void (*render)(HWND,void *,HDC,style_control_owner_action);
} style_control_owner_ops;
/* Caller pins instance/context. Input may consume native handling with0;
 * otherwise native return is preserved. Selection happens after forwarding
 * only while attached. PAINT owns its screen DC on normal renderer return;
 * PRINT and FRAME print DCs stay borrowed, and NULL denotes native border DC.
 * Existing renderer exception behavior is not extended by this operation. */
LRESULT style_control_owner_message(HWND,style_instance *,UINT,WPARAM,LPARAM,const style_control_owner_ops *);
#endif
