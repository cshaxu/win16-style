#ifndef WIN16_CONTROL_SCROLL_MESSAGE_H
#define WIN16_CONTROL_SCROLL_MESSAGE_H
#include "control_state.h"
typedef struct win16_control_scroll_services {
    BOOL (*embedded_input)(HWND,control_style *,UINT,WPARAM,LPARAM);
    BOOL (*input)(HWND,control_style *,UINT,WPARAM,LPARAM);
    void (*finish)(HWND,control_style *,BOOL);
    void (*caret_sync)(HWND,control_style *),(*caret_destroy)(HWND,control_style *);
    void (*draw)(HWND,control_style *,HDC,LONG,BOOL);
    win16_control_state_painter border,scrollbars;
    win16_control_state_present present;
    win16_control_state_painter sizebox;
} win16_control_scroll_services;
LRESULT win16_control_frame_scroll_message(HWND,control_style *,UINT,WPARAM,LPARAM,const win16_control_scroll_services *);
LRESULT win16_control_sizebox_message(HWND,control_style *,UINT,WPARAM,LPARAM,const win16_control_scroll_services *);
LRESULT win16_control_scroll_message(HWND,control_style *,UINT *,WPARAM *,LPARAM *,const win16_control_scroll_services *);
#endif
