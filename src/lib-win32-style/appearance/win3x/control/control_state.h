#ifndef WIN16_CONTROL_STATE_H
#define WIN16_CONTROL_STATE_H
#include "../common/palette.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include "lib-win32-style/application/control/control_state.h"
#include "lib-win32-style/application/control/bindings.h"
#include "lib-win32-style/application/control/scroll_managed.h"
#include "lib-win32-style/application/control/message.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
typedef struct control_style {
    style_instance instance;
    style_control_state state;
    win16_style_palette palette;
    classic_push_colors push_colors;
    classic_choice_colors choice_colors;
    classic_choice_art choice_art;
    HMODULE choice_module;
    style_choice_art_read_fn read_choice_art;
    classic_text_colors label_colors;
    classic_group_colors group_colors;
    classic_edit_border edit_colors[CLASSIC_EDIT_STATE_COUNT];
    classic_list_colors list_colors;
    classic_combo_colors combo_colors;
    classic_combo_arrow_art combo_arrow;
    style_tab_colors tab_colors;
    style_header_colors header_colors;
    classic_progress_colors progress_colors[2];
    classic_trackbar_colors trackbar_colors[2];
    classic_frame_colors frame_colors;
    classic_bar_colors toolbar_colors;
    classic_control_presentation controls;
    classic_surface_colors scroll_arrow_colors;
    style_scroll_arrow_art scroll_art;
    classic_scroll_colors scroll_colors;
    classic_scroll_metrics scroll_metrics;
    style_scroll_policy scroll_policy;
    BOOL themed;
    BOOL file_owner;
    BOOL edit_border;
    BOOL combo_passthrough,common_dialog_combo;
    HWND common_dialog_parent;
    style_control_bindings bindings;
    style_control_updown_art updown;
    BOOL legacy_progress_border;
} control_style;
void win16_control_updown_colors(control_style *);
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
void win16_control_managed_frame_scrollbars_palette_owned(HWND,const win16_style_palette *,const void *,const style_scroll_managed_ops *);
void win16_control_managed_frame_scrollbars_palette(HWND,const win16_style_palette *,const style_scroll_managed_ops *);
void win16_control_scrollbars_paint(HWND,HDC,const win16_style_palette *,SUBCLASSPROC,void (*)(HWND,control_style *,HDC));
#endif
