#ifndef WIN16_CONTROL_FRAME_MESSAGE_H
#define WIN16_CONTROL_FRAME_MESSAGE_H
#include "frame_state.h"
#include "layout.h"
typedef struct win16_control_frame_message_services {
    BOOL (*measure)(HWND,style_window *,const RECT *,win16_style_layout *);
    void (*activate)(HWND,style_window *,const RECT *,BOOL);
    void (*paint)(HWND,style_window *,HDC);
    void (*paint_callback)(HWND,void *,HDC),(*buffered_callback)(HWND,void *,HDC);
    BOOL (*hit_test)(HWND,LPARAM,LRESULT *);
    BOOL (*refresh_font)(HWND,style_window *);
} win16_control_frame_message_services;
BOOL win16_control_frame_message(HWND,style_window *,UINT,WPARAM,LPARAM,const win16_control_frame_message_services *,LRESULT *);
#endif
