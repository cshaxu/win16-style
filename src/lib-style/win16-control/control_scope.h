#ifndef WIN16_CONTROL_SCOPE_H
#define WIN16_CONTROL_SCOPE_H
#include "../win16-common/palette.h"
#include "../common/control_scope.h"
typedef style_control_scope win16_style_control_scope;
typedef struct win16_style_control_counts {unsigned installed,unsupported,failed;} win16_style_control_counts;
typedef struct win16_control_scope_services {
    BOOL (*edit_install)(HWND,BOOL,const win16_style_palette *);
    BOOL (*control_install)(HWND,const win16_style_palette *);
} win16_control_scope_services;
void win16_control_scope_observe(WPARAM,LPARAM,win16_style_control_scope *,const win16_control_scope_services *);
win16_style_control_scope *win16_control_scope_begin(HWND,const win16_style_palette *,win16_style_control_scope **,HHOOK *,HOOKPROC);
BOOL win16_control_scope_end(win16_style_control_scope *,win16_style_control_counts *,win16_style_control_scope **,HHOOK *);
#endif
