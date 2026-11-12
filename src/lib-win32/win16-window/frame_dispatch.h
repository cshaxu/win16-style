#ifndef WIN16_CONTROL_FRAME_DISPATCH_H
#define WIN16_CONTROL_FRAME_DISPATCH_H
#include "frame_message.h"
#include "frame_input.h"
#include "menu_message.h"
#include "lib-win32/style-common/frame.h"
typedef struct win16_control_frame_dispatch_services {
    const win16_control_frame_message_services *message;
    const win16_control_frame_input_services *input;
    win16_control_menu_button_painter paint_button;
    menu_frame **tracking_menu;
    BOOL (*terminal)(HWND);
} win16_control_frame_dispatch_services;
LRESULT win16_control_frame_dispatch(HWND,style_window *,UINT,WPARAM,LPARAM,const win16_control_frame_dispatch_services *);
void win16_control_frame_cancel_repaint(style_window *);
#endif
