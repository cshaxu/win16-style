#ifndef WIN16_CONTROL_FRAME_DISPATCH_H
#define WIN16_CONTROL_FRAME_DISPATCH_H
#include "frame_message.h"
#include "frame_input.h"
#include "menu_message.h"
/* Observed host nonclient draw messages, without interpreting private payloads. */
#define STYLE_NCUAHDRAWCAPTION 0x00ae
#define STYLE_NCUAHDRAWFRAME   0x00af
#define STYLE_UAHDRAWMENU     0x0091
#define STYLE_UAHDRAWMENUITEM 0x0092
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
