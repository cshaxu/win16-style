#ifndef WIN16_CONTROL_DISPATCH_H
#define WIN16_CONTROL_DISPATCH_H
#include "lib-win32-style/application/control/scroll_message.h"
#include "fileowner_message.h"
#include "lib-win32-style/application/control/list.h"
typedef struct win16_control_list_services {
    BOOL (*scroll_input)(HWND,control_style *,UINT,WPARAM,LPARAM);
    win16_control_state_present present;
    win16_control_state_painter paint,border,scrollbars;
} win16_control_list_services;
#include "lib-win32-style/application/control/edit.h"
typedef struct win16_control_edit_services {
    BOOL (*scroll_input)(HWND,control_style *,UINT,WPARAM,LPARAM);
    win16_control_state_painter scrollbars,border,client_border;
} win16_control_edit_services;
typedef struct win16_control_dispatch_services {
    BOOL (*terminal)(HWND);
    BOOL (*class_is)(HWND,const WCHAR *);
    LRESULT (*track_dispatch)(control_style *,HWND,UINT,WPARAM,LPARAM);
    void (*track_unbind)(control_style *);
    BOOL (*is_link)(HWND),(*is_scroll)(HWND),(*is_combo)(HWND),(*is_list)(HWND),(*is_edit)(HWND);
    win16_control_state_present present;
    win16_control_state_painter tab,header,progress,combo,basic,frame_border,sizebox;
    const style_scroll_ops *scroll;
    const win16_control_fileowner_services *fileowner;
    const win16_control_list_services *list;
    const win16_control_edit_services *edit;
} win16_control_dispatch_services;
LRESULT win16_control_dispatch_message(HWND,control_style *,UINT *,WPARAM *,LPARAM *,BOOL,BOOL,BOOL,const win16_control_dispatch_services *);
#endif
