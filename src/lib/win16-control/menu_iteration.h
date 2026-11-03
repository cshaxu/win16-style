#ifndef WIN16_CONTROL_MENU_ITERATION_H
#define WIN16_CONTROL_MENU_ITERATION_H
#include "frame_state.h"
#include "layout.h"
typedef struct win16_control_menu_bar_services {
    int (*open_once)(HWND,style_window *,int,int *);
    BOOL (*layout)(HWND,style_window *,win16_style_layout *);
    style_window *(*lookup)(HWND);
    void (*leave)(HWND,style_window *,BOOL);
    void (*paint)(HWND,style_window *);
    void (*select)(HWND,style_window *,int);
} win16_control_menu_bar_services;
void win16_control_open_menu_bar(HWND,style_window *,int,const win16_control_menu_bar_services *);
#endif
