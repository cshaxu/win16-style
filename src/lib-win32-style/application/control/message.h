#ifndef STYLE_CONTROL_MESSAGE_H
#define STYLE_CONTROL_MESSAGE_H
#include "bindings.h"
#include "guard.h"
/* Synchronous message transaction; native argument rewrites and temporary
 * font leases belong to this object. Providers may change these values but
 * must not retain the object. Window data and rendering stay caller-owned. */
typedef struct style_control_message {
    UINT message;WPARAM parameter;LPARAM value;
    HFONT previous,outbound;
    BOOL tab,header,progress,native_font;
} style_control_message;
typedef struct style_control_message_ops {
    void *context;style_control_guard_ops guard;
    BOOL (*prepare_font)(HWND,void *,style_control_message *);
    LRESULT (*dispatch)(HWND,void *,style_control_message *);
    style_control_state *(*lookup)(HWND);
} style_control_message_ops;
/* Pins instance and font until the existing normal-return completion. Native
 * guard/draw operations retain their own existing exception contracts; this
 * coordinator adds no new catch or recovery around provider callbacks. */
LRESULT style_control_message_dispatch(HWND,style_instance *,style_control_state *,style_control_bindings *,UINT,WPARAM,LPARAM,const style_control_message_ops *);
#endif
