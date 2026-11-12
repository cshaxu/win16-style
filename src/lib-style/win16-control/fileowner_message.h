#ifndef WIN16_CONTROL_FILEOWNER_MESSAGE_H
#define WIN16_CONTROL_FILEOWNER_MESSAGE_H
#include "control_state.h"
typedef struct win16_control_fileowner_services {
    BOOL (*scroll_input)(HWND,control_style *,UINT,WPARAM,LPARAM);
    win16_control_state_painter combo,border,scrollbars;
} win16_control_fileowner_services;
LRESULT win16_control_fileowner_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,const win16_control_fileowner_services *services);
#endif
