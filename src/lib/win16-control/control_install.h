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
#endif
