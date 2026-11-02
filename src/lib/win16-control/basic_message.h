#ifndef WIN16_CONTROL_BASIC_MESSAGE_H
#define WIN16_CONTROL_BASIC_MESSAGE_H
#include "control_state.h"
LRESULT win16_control_basic_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,win16_control_state_present present,win16_control_state_painter paint);
#endif
