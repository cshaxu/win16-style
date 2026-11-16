#ifndef WIN16_CONTROL_EDIT_MESSAGE_H
#define WIN16_CONTROL_EDIT_MESSAGE_H
#include "control_state.h"
typedef struct win16_control_edit_services {
    BOOL (*scroll_input)(HWND,control_style *,UINT,WPARAM,LPARAM);
    win16_control_state_painter scrollbars,border,client_border;
} win16_control_edit_services;
LRESULT win16_control_edit_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,const win16_control_edit_services *services);
#endif
