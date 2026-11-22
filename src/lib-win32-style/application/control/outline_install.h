#ifndef STYLE_CONTROL_OUTLINE_INSTALL_H
#define STYLE_CONTROL_OUTLINE_INSTALL_H
#include "../window/palette.h"
#include <commctrl.h>
#include "combo.h"
#include "control_state.h"
#include "lib-win32-style/application/control/control_theme.h"
typedef struct style_outline_control_install_services {
    SUBCLASSPROC control_proc;
    const style_control_combo_ops *combo;
    BOOL (*combo_candidate)(HWND);
    BOOL (*get_combo_info)(HWND,PCOMBOBOXINFO);
    BOOL (*install_control)(HWND,const style_window_palette *);
    BOOL (*remove_control)(HWND);
    style_control_theme_load load_theme;
    const style_control_fonts *fonts;
} style_outline_control_install_services;
BOOL style_outline_control_install(HWND,const style_window_palette *,BOOL,BOOL,BOOL,int,const int *,const void *,const BOOL *,HWND,
    const style_outline_control_install_services *);

typedef struct style_outline_control_special_services {
    SUBCLASSPROC control_proc,trackbar_notify,toolbar_notify;
    BOOL (*frame_install)(HWND,const style_window_palette *,const void *,const BOOL *);
    BOOL (*remove_owned)(HWND,const void *);
    BOOL (*theme_retained)(HWND),(*theme_capture)(HWND),(*theme_enable)(HWND,const void *);
} style_outline_control_special_services;
BOOL style_outline_control_special_updown_install(HWND,const style_window_palette *,const void *,const BOOL *,const style_outline_control_special_services *);
BOOL style_outline_control_special_trackbar_install(HWND,const style_window_palette *,const void *,const BOOL *,const style_outline_control_special_services *);
BOOL style_outline_control_special_toolbar_install(HWND,const style_window_palette *,const void *,const BOOL *,const style_outline_control_special_services *);
BOOL style_outline_control_special_progress_install(HWND,const style_window_palette *,const void *,const BOOL *,const style_outline_control_special_services *);
BOOL style_outline_control_special_legacy_progress_border_install(HWND,const style_window_palette *,const void *,const BOOL *,const style_outline_control_special_services *);
BOOL style_outline_control_special_tree_install(HWND,const style_window_palette *,const void *,const BOOL *,const style_outline_control_special_services *);
BOOL style_outline_control_special_grid_install(HWND,const style_window_palette *,const void *,const BOOL *,const style_outline_control_special_services *);
BOOL style_outline_control_special_picture_install(HWND,const style_window_palette *,const void *,const BOOL *,const style_outline_control_special_services *);
#endif
