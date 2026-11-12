#ifndef WIN16_CONTROL_SPECIAL_INSTALL_H
#define WIN16_CONTROL_SPECIAL_INSTALL_H
#include "control_state.h"
#include <commctrl.h>
typedef struct win16_control_special_services {
    SUBCLASSPROC control_proc,trackbar_notify,toolbar_notify;
    BOOL (*updown_acquire)(void *);void (*updown_release)(void *);
    BOOL (*frame_install)(HWND,const win16_style_palette *,const void *,const BOOL *);
    BOOL (*remove_owned)(HWND,const void *);
    BOOL (*theme_retained)(HWND),(*theme_capture)(HWND),(*theme_enable)(HWND,const void *);
} win16_control_special_services;
BOOL win16_control_special_updown_install(HWND,const win16_style_palette *,const void *,const BOOL *,const win16_control_special_services *);
BOOL win16_control_special_trackbar_install(HWND,const win16_style_palette *,const void *,const BOOL *,const win16_control_special_services *);
BOOL win16_control_special_toolbar_install(HWND,const win16_style_palette *,const void *,const BOOL *,const win16_control_special_services *);
BOOL win16_control_special_progress_install(HWND,const win16_style_palette *,const void *,const BOOL *,const win16_control_special_services *);
BOOL win16_control_special_legacy_progress_border_install(HWND,const win16_style_palette *,const void *,const BOOL *,const win16_control_special_services *);
BOOL win16_control_special_tree_install(HWND,const win16_style_palette *,const void *,const BOOL *,const win16_control_special_services *);
BOOL win16_control_special_grid_install(HWND,const win16_style_palette *,const void *,const BOOL *,const win16_control_special_services *);
BOOL win16_control_special_picture_install(HWND,const win16_style_palette *,const void *,const BOOL *,const win16_control_special_services *);
#endif
