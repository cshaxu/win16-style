#ifndef WIN16_CONTROL_MENU_MESSAGE_H
#define WIN16_CONTROL_MENU_MESSAGE_H
#include "frame_state.h"
typedef void (*win16_control_menu_button_painter)(HWND,style_window *,int);
BOOL win16_control_menu_message(HWND,style_window *,UINT,WPARAM,LPARAM,win16_control_menu_button_painter,LRESULT *);
#endif
