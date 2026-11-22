#ifndef STYLE_CONTROL_GUARD_H
#define STYLE_CONTROL_GUARD_H
#include "basic.h"
/* Complete caller qualifications, evaluated in order. The instance pins state
 * through native forwarding and retirement; font source is this state's
 * existing provider. A failed outer removal only falls back for the native
 * owner-draw transition, as in the original protocol. */
typedef struct style_control_guard_ops {
    BOOL (*valid)(HWND),(*dormant)(HWND),(*native_button)(HWND),(*unsupported_choice)(HWND);
    BOOL (*remove_window)(HWND),(*remove_control)(HWND);
} style_control_guard_ops;
BOOL style_control_guard_message(HWND,style_instance *,style_control_state *,UINT,WPARAM,LPARAM,const style_control_guard_ops *,LRESULT *);

BOOL style_control_outline_supported(HWND,HWND);
style_control_basic_kind style_control_outline_basic_kind(HWND);
BOOL style_control_outline_choice_unsupported(HWND);
#endif
