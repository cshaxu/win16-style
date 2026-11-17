#ifndef WIN16_CONTROL_SCOPE_H
#define WIN16_CONTROL_SCOPE_H
#include "../common/palette.h"
#include "../../../application/control/control_scope.h"
typedef style_control_scope win16_style_control_scope;
typedef style_control_scope_counts win16_style_control_counts;
typedef struct win16_control_scope_services {
    BOOL (*edit_install)(HWND,BOOL,const win16_style_palette *);
    BOOL (*control_install)(HWND,const win16_style_palette *);
} win16_control_scope_services;
BOOL win16_control_scope_install(style_control_scope *,const CWPRETSTRUCT *,BOOL edit,void *services);
win16_style_control_scope *win16_control_scope_begin(HWND,const win16_style_palette *,style_control_scope_manager *,HOOKPROC);
#endif
