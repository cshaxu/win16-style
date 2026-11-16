#ifndef STYLE_CONTROL_LIST_MESSAGE_H
#define STYLE_CONTROL_LIST_MESSAGE_H
#include "control_state.h"
/* Caller pins the instance across this dispatch. All render operations borrow
 * their HDC; input may synchronously retire the instance. */
typedef struct style_control_list_ops {
    void *context;
    BOOL suppress_hover;
    BOOL (*input)(HWND,void *,UINT,WPARAM,LPARAM);
    void (*draw)(HWND,void *,HDC,BOOL buffered);
    void (*border)(HWND,void *,HDC);
    void (*scrollbars)(HWND,void *,HDC);
} style_control_list_ops;
LRESULT style_control_list_message(HWND window,style_instance *instance,style_control_state *state,
    UINT message,WPARAM wparam,LPARAM lparam,const style_control_list_ops *ops);
#endif
