#ifndef STYLE_CONTROL_OUTLINE_DISPATCH_H
#define STYLE_CONTROL_OUTLINE_DISPATCH_H
#include "lib-win32-style/application/control/scroll_message.h"
#include "lib-win32-style/application/control/outline_state.h"

typedef struct style_outline_control_fileowner_services {
    BOOL (*scroll_input)(HWND,style_outline_control *,UINT,WPARAM,LPARAM);
    style_outline_control_state_painter combo,border,scrollbars;
} style_outline_control_fileowner_services;
#include "lib-win32-style/application/control/list.h"
typedef struct style_outline_control_list_services {
    BOOL (*scroll_input)(HWND,style_outline_control *,UINT,WPARAM,LPARAM);
    style_outline_control_state_painter paint,border,scrollbars;
} style_outline_control_list_services;
#include "lib-win32-style/application/control/edit.h"
typedef struct style_outline_control_edit_services {
    BOOL (*scroll_input)(HWND,style_outline_control *,UINT,WPARAM,LPARAM);
    style_outline_control_state_painter scrollbars,border,client_border;
} style_outline_control_edit_services;
typedef struct style_outline_control_dispatch_services {
    BOOL (*terminal)(HWND);
    style_control_track_calls *track_calls;SUBCLASSPROC track_notify;
    style_outline_control_state_painter combo,basic,frame_border,sizebox;
    const style_scroll_ops *scroll;
    const style_outline_control_fileowner_services *fileowner;
    const style_outline_control_list_services *list;
    const style_outline_control_edit_services *edit;
} style_outline_control_dispatch_services;
LRESULT style_outline_control_dispatch_message(HWND,style_outline_control *,UINT *,WPARAM *,LPARAM *,BOOL,BOOL,BOOL,const style_outline_control_dispatch_services *);
#endif
