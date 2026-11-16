#ifndef WIN16_CONTROL_DISPATCH_H
#define WIN16_CONTROL_DISPATCH_H
#include "scroll_message.h"
#include "fileowner_message.h"
#include "list_message.h"
#include "edit_message.h"
typedef struct win16_control_dispatch_services {
    BOOL (*terminal)(HWND);
    BOOL (*class_is)(HWND,const WCHAR *);
    LRESULT (*track_dispatch)(control_style *,HWND,UINT,WPARAM,LPARAM);
    void (*track_unbind)(control_style *);
    BOOL (*is_link)(HWND),(*is_scroll)(HWND),(*is_combo)(HWND),(*is_list)(HWND),(*is_edit)(HWND);
    win16_control_state_present present;
    win16_control_state_painter tab,header,progress,combo,basic;
    const win16_control_scroll_services *scroll;
    const win16_control_fileowner_services *fileowner;
    const win16_control_list_services *list;
    const win16_control_edit_services *edit;
} win16_control_dispatch_services;
LRESULT win16_control_dispatch_message(HWND,control_style *,UINT *,WPARAM *,LPARAM *,BOOL,BOOL,BOOL,const win16_control_dispatch_services *);
#endif
