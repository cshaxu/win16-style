#ifndef WIN16_CONTROL_FRAME_LIFECYCLE_H
#define WIN16_CONTROL_FRAME_LIFECYCLE_H
#include "frame_state.h"
#include "layout.h"
#include <commctrl.h>
typedef struct win16_control_frame_lifecycle_services {
    SUBCLASSPROC style_proc;
    HRESULT (WINAPI *get_window_attribute)(HWND,DWORD,PVOID,DWORD);
    HRESULT (WINAPI *set_window_attribute)(HWND,DWORD,LPCVOID,DWORD);
    BOOL (*local_window)(HWND);
    style_window *(*state)(HWND);
    BOOL (*refresh_font)(HWND,style_window *),(*layout)(HWND,style_window *,win16_style_layout *);
    void (*activate_frame)(HWND,style_window *,const RECT *,BOOL);
    BOOL (*scrollbars_install)(HWND,const win16_style_palette *,const void *,const BOOL *);
    void (*scrollbars_palette)(HWND,const win16_style_palette *,const void *);
    BOOL (*control_remove)(HWND,const void *),(*remove)(HWND);
    void (*leave_bar)(HWND,style_window *,BOOL),(*paint)(HWND,style_window *,HDC);
} win16_control_frame_lifecycle_services;
BOOL win16_control_frame_install(HWND,const win16_style_palette *,BOOL,const void *,const BOOL *,const win16_control_frame_lifecycle_services *);
BOOL win16_control_menu_owner_install(HWND,const void *,const BOOL *,const win16_control_frame_lifecycle_services *);
BOOL win16_control_frame_remove_owned(HWND,const void *,const win16_control_frame_lifecycle_services *);
BOOL win16_control_frame_remove(HWND,const win16_control_frame_lifecycle_services *);
BOOL win16_control_frame_set_palette(HWND,const win16_style_palette *,const win16_control_frame_lifecycle_services *);
BOOL win16_control_frame_terminal(HWND,const win16_control_frame_lifecycle_services *);
#endif
