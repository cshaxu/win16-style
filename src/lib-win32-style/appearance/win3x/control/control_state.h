#ifndef WIN16_CONTROL_STATE_H
#define WIN16_CONTROL_STATE_H
#include "../common/palette.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include "lib-win32-style/application/control/control_state.h"
#include "lib-win32-style/application/control/combo.h"
#include "lib-win32-style/application/control/scroll_controller.h"
#include "updown_adapter.h"
typedef struct control_style {
    style_instance instance;
    style_control_state state;
    win16_style_palette palette;
    BOOL file_owner;
    BOOL edit_border;
    style_control_combo combo;
    BOOL combo_passthrough,common_dialog_combo;
    HWND common_dialog_parent;
    style_scroll_controller scroll;
    unsigned tab_epoch;
    unsigned header_epoch,progress_epoch;
    HWND track_parent,track_window;
    BOOL track_internal;
    HWND toolbar_parent,toolbar_window;
    BOOL toolbar_internal;
    ntw16s_updown_paint updown;
    int header_pressed;
    BOOL legacy_progress_border,list_theme_owned;
} control_style;
control_style *win16_control_state_create(void);
void win16_control_state_dispose(void *context);
BOOL win16_control_state_prepare_font(HWND window,control_style *state,UINT message,WPARAM *parameter,BOOL native_font,HFONT *previous,HFONT *outbound);
typedef void (*win16_control_state_painter)(HWND,control_style *,HDC);
typedef void (*win16_control_state_present)(HWND,control_style *,HDC,win16_control_state_painter);
LRESULT win16_control_tab_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,win16_control_state_present present,win16_control_state_painter paint);
LRESULT win16_control_header_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,win16_control_state_present present,win16_control_state_painter paint);
LRESULT win16_control_progress_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,win16_control_state_present present,win16_control_state_painter paint);
#endif
