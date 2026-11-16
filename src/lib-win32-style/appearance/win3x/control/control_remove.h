#ifndef WIN16_CONTROL_REMOVE_H
#define WIN16_CONTROL_REMOVE_H
#include "control_state.h"
#include <commctrl.h>
typedef struct win16_control_remove_services {
    SUBCLASSPROC control_proc;
    void (*scroll_finish)(HWND,control_style *,BOOL);
    void (*trackbar_unbind)(control_style *),(*toolbar_unbind)(control_style *);
    void (*combo_text_unbind)(control_style *,BOOL),(*detach_combo_list)(control_style *);
    void (*theme_remove)(HWND,const void *);
    BOOL (*theme_terminal)(HWND,const void *);
    void (*scroll_caret_destroy)(HWND,control_style *),(*scroll_stop_timer)(HWND,control_style *),(*scroll_release_capture)(HWND,control_style *);
    BOOL (*terminal_combo_list)(control_style *);
} win16_control_remove_services;
BOOL win16_control_remove(HWND,const win16_control_remove_services *);
BOOL win16_control_terminal(HWND,const win16_control_remove_services *);
BOOL win16_control_combo_list_retire(control_style *,SUBCLASSPROC,BOOL (*retire)(HWND));
#endif
