#ifndef STYLE_CLIENT_FRAME_INPUT_H
#define STYLE_CLIENT_FRAME_INPUT_H
#include "lib-win32-style/application/window/client_frame_renderer.h"
#include "client_frame_state.h"
#include "lib-win32-style/application/menu/bar_controller.h"
typedef struct style_client_frame_input_services {
    BOOL (*layout)(HWND,style_client_frame *,style_full_client_layout *);
    style_menu_bar_target (*menu)(HWND,style_client_frame *);
    style_client_frame_button_view (*button_view)(style_client_frame *);
} style_client_frame_input_services;
BOOL style_client_frame_input(HWND,style_client_frame *,UINT,WPARAM,LPARAM,const style_client_frame_input_services *,LRESULT *);
#endif
