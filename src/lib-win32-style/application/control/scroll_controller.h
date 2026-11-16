#ifndef STYLE_SCROLL_CONTROLLER_H
#define STYLE_SCROLL_CONTROLLER_H
#include "control_state.h"
#include "scroll_resources.h"
#include "scroll_state.h"
#include "scroll_track.h"

typedef struct style_scroll_controller {
    style_scroll_track track;
    style_scroll_resources resources;
    unsigned epoch;
    BOOL reporting,key;
    LONG report,object;
} style_scroll_controller;

/* Synchronous complete operations. Client layout accepts a NULL origin;
 * embedded layout returns a screen origin. Caller pins the instance across
 * input/notifications; operations and context are borrowed for that call. */
typedef struct style_scroll_ops {
    void *context;
    const style_scroll_policy *policy;
    style_scroll_info_fn get_info;
    BOOL (*layout)(HWND,void *,LONG,style_scroll_layout *,POINT *);
    void (*draw)(HWND,void *,HDC,LONG,BOOL);
    void (*nonclient)(HWND,void *,HDC);
    void (*caret)(HWND,void *);
} style_scroll_ops;

BOOL style_scroll_control_input(HWND,style_instance *,style_scroll_controller *,const style_scroll_ops *,UINT,WPARAM,LPARAM);
BOOL style_scroll_embedded_input(HWND,style_instance *,const style_control_state *,style_scroll_controller *,const style_scroll_ops *,UINT,WPARAM,LPARAM);
void style_scroll_finish(HWND,style_instance *,style_scroll_controller *,const style_scroll_ops *,BOOL);
void style_scroll_apply_actions(HWND,style_instance *,style_scroll_controller *,const style_scroll_ops *,const style_scroll_actions *,unsigned);
#endif
