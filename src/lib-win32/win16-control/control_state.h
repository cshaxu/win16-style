#ifndef WIN16_CONTROL_STATE_H
#define WIN16_CONTROL_STATE_H
#include "../win16-common/palette.h"
#include "control_protocol.h"
#include "scroll_track.h"
#include "updown_adapter.h"
typedef struct control_style {
    win16_style_palette palette;
    unsigned calls;
    BOOL detached,removing,file_owner;
    HFONT font,original_font;
    BOOL edit_border;
    HWND combo_list,combo_edit,combo_parent;
    HFONT combo_edit_font,combo_edit_source,combo_fonts[32];
    unsigned combo_font_count,combo_paints;
    BOOL combo_bound,combo_passthrough,common_dialog_combo;
    HWND common_dialog_parent;
    struct control_style *owner;
    const void *creation_owner;
    ntw16s_control_protocol *protocol;
    win16_scroll_track scroll;
    UINT_PTR scroll_timer;
    unsigned scroll_epoch;
    unsigned tab_epoch;
    unsigned header_epoch,progress_epoch;
    HWND track_parent,track_window;
    BOOL track_internal;
    HWND toolbar_parent,toolbar_window;
    BOOL toolbar_internal;
    ntw16s_updown_paint updown;
    int header_pressed;
    BOOL scroll_reporting,scroll_key;
    LONG scroll_report;
    BOOL list_scrollbar,edit_scrollbars,frame_scrollbars,native_border,list_redraw_off,legacy_progress_border,list_theme_owned;
    unsigned list_suppression;
    HWND list_capture_before;
    LONG embedded_object;
    BOOL scroll_caret,scroll_caret_shown;
    SIZE scroll_caret_size;
    int tab_count,*tab_positions;
} control_style;
control_style *win16_control_state_create(void);
void win16_control_state_dispose(control_style *state);
BOOL win16_control_state_prepare_font(HWND window,control_style *state,UINT message,WPARAM *parameter,BOOL native_font,HFONT *previous,HFONT *outbound);
typedef void (*win16_control_state_painter)(HWND,control_style *,HDC);
typedef void (*win16_control_state_present)(HWND,control_style *,HDC,win16_control_state_painter);
BOOL win16_control_state_can_suppress_redraw(HWND window,const control_style *state);
LRESULT win16_control_combo_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,win16_control_state_present present,win16_control_state_painter paint);
LRESULT win16_control_tab_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,win16_control_state_present present,win16_control_state_painter paint);
LRESULT win16_control_header_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,win16_control_state_present present,win16_control_state_painter paint);
LRESULT win16_control_progress_message(HWND window,control_style *state,UINT message,WPARAM wparam,LPARAM lparam,win16_control_state_present present,win16_control_state_painter paint);
#endif
