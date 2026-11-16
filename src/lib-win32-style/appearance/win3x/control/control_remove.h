#ifndef WIN16_CONTROL_REMOVE_H
#define WIN16_CONTROL_REMOVE_H
#include "control_state.h"
#include <commctrl.h>
typedef struct win16_control_remove_services {
    SUBCLASSPROC control_proc;
    void (*scroll_finish)(HWND,control_style *,BOOL);
    void (*trackbar_unbind)(control_style *),(*toolbar_unbind)(control_style *);
    style_control_state *(*lookup)(HWND);
    BOOL (*remove_list)(HWND);
    void (*theme_remove)(HWND,const void *);
    BOOL (*theme_terminal)(HWND,const void *);
    BOOL (*terminal_list)(HWND);
} win16_control_remove_services;
BOOL win16_control_remove(HWND,const win16_control_remove_services *);
BOOL win16_control_terminal(HWND,const win16_control_remove_services *);
#endif
