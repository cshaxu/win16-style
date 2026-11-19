#ifndef WIN16_CONTROL_STATE_H
#define WIN16_CONTROL_STATE_H
#include "../common/palette.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include "lib-win32-style/application/control/control_state.h"
#include "lib-win32-style/application/control/bindings.h"
#include "updown_paint.h"
#include "lib-win32-style/application/control/message.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
typedef struct control_style {
    style_instance instance;
    style_control_state state;
    win16_style_palette palette;
    classic_push_colors push_colors;
    classic_surface_colors scroll_arrow_colors;
    classic_scroll_colors scroll_colors;
    classic_scroll_metrics scroll_metrics;
    BOOL themed;
    BOOL file_owner;
    BOOL edit_border;
    BOOL combo_passthrough,common_dialog_combo;
    HWND common_dialog_parent;
    style_control_bindings bindings;
    win16_control_updown updown;
    BOOL legacy_progress_border;
} control_style;
control_style *win16_control_state_create(void);
void win16_control_state_dispose(void *context);
BOOL win16_control_state_prepare_font(HWND,void *,style_control_message *);
typedef void (*win16_control_state_painter)(HWND,control_style *,HDC);typedef struct win16_control_remove_services {
    SUBCLASSPROC control_proc;
    void (*scroll_finish)(HWND,control_style *,BOOL);
    SUBCLASSPROC track_notify,toolbar_notify;
    style_control_state *(*lookup)(HWND);
    BOOL (*remove_list)(HWND);
    void (*theme_remove)(HWND,const void *);
    BOOL (*theme_terminal)(HWND,const void *);
    BOOL (*terminal_list)(HWND);
} win16_control_remove_services;
BOOL win16_control_remove(HWND,const win16_control_remove_services *);
BOOL win16_control_terminal(HWND,const win16_control_remove_services *);
#endif
