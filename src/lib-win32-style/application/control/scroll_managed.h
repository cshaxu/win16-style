#ifndef STYLE_SCROLL_MANAGED_H
#define STYLE_SCROLL_MANAGED_H
#include "scroll_controller.h"
typedef struct style_scroll_binding {
    style_instance *instance;
    style_scroll_controller *scroll;
} style_scroll_binding;
typedef enum style_scroll_role {STYLE_SCROLL_LIST,STYLE_SCROLL_EDIT} style_scroll_role;
/* lookup returns both borrowed objects, or an empty binding. draw receives the
 * instance's opaque context. No private object layout is known by this layer. */
typedef struct style_scroll_managed_ops {
    style_scroll_binding (*lookup)(HWND);
    BOOL (*remove)(HWND);
    style_scroll_info_fn get_info;
    int (*set_info)(HWND,int,LPCSCROLLINFO,BOOL);
    style_scroll_painter draw;
    BOOL (*eligible)(HWND,style_scroll_role);
} style_scroll_managed_ops;
BOOL style_scroll_managed_active(HWND,const style_scroll_managed_ops *);
BOOL style_scroll_managed_get_info(HWND,int,LPSCROLLINFO,const style_scroll_managed_ops *);
int style_scroll_managed_set_info(HWND,int,LPCSCROLLINFO,BOOL,const style_scroll_managed_ops *);
BOOL style_scroll_managed_enable(HWND,style_scroll_role,const style_scroll_managed_ops *);
void style_scroll_managed_frame_remove(HWND,const style_scroll_managed_ops *);
#endif
