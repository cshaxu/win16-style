#ifndef STYLE_SCROLL_MESSAGE_H
#define STYLE_SCROLL_MESSAGE_H
#include "basic.h"
#include "scroll_controller.h"
/* Complete profile rendering operations; context is borrowed for dispatch.
 * Native-border ownership is controller state, not a copied profile flag. */
typedef struct style_scroll_frame_ops {
    void *context;
    BOOL suppress_hover;
    void (*border)(HWND,void *,HDC);
    void (*overlay)(HWND,void *,HDC);
} style_scroll_frame_ops;
LRESULT style_scroll_frame_message(HWND,style_instance *,const style_control_state *,style_scroll_controller *,UINT,WPARAM,LPARAM,const style_scroll_ops *,const style_scroll_frame_ops *);
LRESULT style_scroll_sizebox_message(HWND,UINT,WPARAM,LPARAM,const style_control_draw *);
LRESULT style_scroll_control_message(HWND,style_instance *,style_scroll_controller *,UINT *,WPARAM *,LPARAM *,const style_scroll_ops *);
#endif
