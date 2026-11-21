#ifndef WIN16_CONTROL_FRAME_INPUT_H
#define WIN16_CONTROL_FRAME_INPUT_H
#include "frame_renderer.h"
#include "frame_state.h"
#include "lib-win32-style/application/menu/bar_controller.h"
typedef struct win16_control_frame_input_services {
    BOOL (*layout)(HWND,style_window *,style_full_client_layout *);
    style_menu_bar_target (*menu)(HWND,style_window *);
    win16_control_frame_button_view (*button_view)(style_window *);
} win16_control_frame_input_services;
BOOL win16_control_frame_input(HWND,style_window *,UINT,WPARAM,LPARAM,const win16_control_frame_input_services *,LRESULT *);
#endif
