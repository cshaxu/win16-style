#ifndef WIN16_CONTROL_MENU_ITERATION_H
#define WIN16_CONTROL_MENU_ITERATION_H
#include "frame_state.h"
#include "../win16-common/layout.h"
typedef struct menu_frame {
    HWND window;
    RECT button;
    BOOL double_click,escape;
    LPARAM point;
    HMENU root,active;
    int direction;
    style_window *style;
    int selected,target;
    struct menu_frame *previous;
    HHOOK hook;
} menu_frame;
typedef struct win16_control_menu_bar_services {
    menu_frame *(*current)(void);
    void (*begin)(menu_frame *);
    void (*end)(menu_frame *);
    BOOL (*layout)(HWND,style_window *,win16_style_layout *);
    style_window *(*lookup)(HWND);
    void (*leave)(HWND,style_window *,BOOL);
    void (*paint)(HWND,style_window *);
    void (*select)(HWND,style_window *,int);
} win16_control_menu_bar_services;
void win16_control_open_menu_bar(HWND,style_window *,int,const win16_control_menu_bar_services *);
#endif
