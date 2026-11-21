#ifndef STYLE_CLIENT_FRAME_LIFECYCLE_H
#define STYLE_CLIENT_FRAME_LIFECYCLE_H
#include "client_frame_state.h"
#include "lib-win32-style/application/window/full_client_layout.h"
#include <commctrl.h>
typedef struct style_client_frame_lifecycle_services {
    SUBCLASSPROC style_proc;
    HRESULT (WINAPI *get_window_attribute)(HWND,DWORD,PVOID,DWORD);
    HRESULT (WINAPI *set_window_attribute)(HWND,DWORD,LPCVOID,DWORD);
    BOOL (*local_window)(HWND);
    style_client_frame *(*state)(HWND);
    /* Supplies resolved presentation before attachment. A menu-only frame
     * also needs the provider's menu font lease populated here. */
    DWORD (*load_theme)(style_client_frame *,BOOL use_theme_colors);
    BOOL (*refresh_font)(HWND,void *);
    BOOL (*layout)(HWND,style_client_frame *,style_full_client_layout *);
    void (*activate_frame)(HWND,void *,const RECT *,BOOL);
    BOOL (*scrollbars_install)(HWND,const style_window_palette *,const void *,const BOOL *);
    BOOL (*control_remove)(HWND,const void *),(*remove)(HWND);
    void (*leave_bar)(HWND,style_client_frame *,BOOL);
} style_client_frame_lifecycle_services;
BOOL style_client_frame_install(HWND,const style_window_palette *,BOOL,const void *,const BOOL *,const style_client_frame_lifecycle_services *);
BOOL style_client_frame_menu_owner_install(HWND,const void *,const BOOL *,const style_client_frame_lifecycle_services *);
BOOL style_client_frame_remove_owned(HWND,const void *,const style_client_frame_lifecycle_services *);
BOOL style_client_frame_remove(HWND,const style_client_frame_lifecycle_services *);
BOOL style_client_frame_terminal(HWND,const style_client_frame_lifecycle_services *);
#endif
