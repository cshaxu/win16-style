#ifndef WIN16_CONTROL_LIST_MESSAGE_H
#define WIN16_CONTROL_LIST_MESSAGE_H
#include "control_state.h"
typedef struct win16_control_list_services {
    BOOL (*scroll_input)(HWND,control_style *,UINT,WPARAM,LPARAM);
    LRESULT (*set_tabs)(HWND,control_style *,WPARAM,LPARAM);
    win16_control_state_present present;
    win16_control_state_painter paint,border,scrollbars;
} win16_control_list_services;
LRESULT win16_control_list_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,const win16_control_list_services *services);
#endif
