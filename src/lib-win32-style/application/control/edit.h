#ifndef STYLE_CONTROL_EDIT_MESSAGE_H
#define STYLE_CONTROL_EDIT_MESSAGE_H
#include "control_state.h"
#include "scroll_resources.h"
/* Caller pins instance and state. Native EDIT owns buffer, selection, undo and
 * IME; callbacks borrow DCs and may synchronously retire this attachment.
 * NULL scroll retains native scrolling; each omitted operation retains that
 * native part. Profiles do not need no-op callbacks for unselected parts. */
typedef struct style_control_edit_ops {
    void *context;
    BOOL suppress_hover;
    BOOL (*input)(HWND,void *,UINT,WPARAM,LPARAM);
    void (*scrollbars)(HWND,void *,HDC);
    void (*border)(HWND,void *,HDC);
    void (*client_border)(HWND,void *,HDC);
} style_control_edit_ops;
LRESULT style_control_edit_message(HWND window,style_instance *instance,style_control_state *state,
    const style_scroll_resources *scroll,UINT message,WPARAM parameter,LPARAM value,const style_control_edit_ops *ops);
#endif
