#ifndef STYLE_CLIENT_FRAME_DISPATCH_H
#define STYLE_CLIENT_FRAME_DISPATCH_H
#include "lib-win32-style/application/window/messages.h"
#include "client_frame_input.h"
#include "lib-win32-style/application/window/frame.h"
typedef void (*style_client_frame_menu_button_painter)(HWND,style_client_frame *,int);

typedef struct style_client_frame_dispatch_services {
    const style_frame_message_ops *message;
    const style_client_frame_input_services *input;
    style_client_frame_menu_button_painter paint_button;
    BOOL (*terminal)(HWND);
} style_client_frame_dispatch_services;
LRESULT style_client_frame_dispatch(HWND,style_client_frame *,UINT,WPARAM,LPARAM,const style_client_frame_dispatch_services *);
#endif
