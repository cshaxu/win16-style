#ifndef WIN16_CONTROL_INSTALL_H
#define WIN16_CONTROL_INSTALL_H
#include "control_state.h"
#include <commctrl.h>
typedef struct win16_control_install_services {
    SUBCLASSPROC control_proc,combo_proc;
    BOOL (*combo_candidate)(HWND);
    BOOL (*get_combo_info)(HWND,PCOMBOBOXINFO);
    BOOL (*install_control)(HWND,const win16_style_palette *);
    BOOL (*remove_control)(HWND);
} win16_control_install_services;
BOOL win16_control_install(HWND,const win16_style_palette *,BOOL,BOOL,BOOL,int,const int *,const void *,const BOOL *,HWND,
    const win16_control_install_services *);
typedef struct win16_control_dialog_combo_services {
    BOOL (*install)(HWND,const win16_style_palette *,BOOL,BOOL,BOOL,int,const int *,const void *,const BOOL *);
    BOOL (*get_combo_info)(HWND,PCOMBOBOXINFO);
    BOOL (*grid_install)(HWND,const win16_style_palette *,const void *,const BOOL *);
    BOOL (*list_enable)(HWND),(*is_edit)(HWND);
    BOOL (*control_install)(HWND,BOOL,BOOL,BOOL,const win16_style_palette *,const void *,const BOOL *);
} win16_control_dialog_combo_services;
BOOL win16_control_dialog_combo_install(HWND,const void *,HWND *,const win16_control_dialog_combo_services *);
#endif
