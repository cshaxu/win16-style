#ifndef WIN16_CONTROL_FRAME_DISPATCH_H
#define WIN16_CONTROL_FRAME_DISPATCH_H
#include "lib-win32-style/application/window/messages.h"
#include "frame_input.h"
#include "lib-win32-style/application/window/frame.h"
typedef void (*win16_control_menu_button_painter)(HWND,style_window *,int);

typedef struct win16_control_frame_dispatch_services {
    const style_frame_message_ops *message;
    const win16_control_frame_input_services *input;
    win16_control_menu_button_painter paint_button;
    BOOL (*terminal)(HWND);
} win16_control_frame_dispatch_services;
LRESULT win16_control_frame_dispatch(HWND,style_window *,UINT,WPARAM,LPARAM,const win16_control_frame_dispatch_services *);
#endif
