#ifndef WIN16_CONTROL_FRAME_INPUT_H
#define WIN16_CONTROL_FRAME_INPUT_H
#include "frame_view.h"
#include "menu_iteration.h"
typedef struct win16_control_frame_input_services {
    menu_frame *(*current)(void);
    void (*begin)(menu_frame *),(*end)(menu_frame *);
    BOOL (*layout)(HWND,style_window *,win16_style_layout *);
    int (*bar_hit)(HWND,style_window *,POINT);
    void (*paint)(HWND,style_window *);
    void (*select)(HWND,style_window *,int);
    void (*leave)(HWND,style_window *,BOOL);
    void (*open)(HWND,style_window *,int);
    win16_control_frame_button_view (*button_view)(style_window *);
} win16_control_frame_input_services;
BOOL win16_control_frame_input(HWND,style_window *,UINT,WPARAM,LPARAM,const win16_control_frame_input_services *,LRESULT *);
#endif
