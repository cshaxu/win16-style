#ifndef WIN16_CONTROL_FRAME_STATE_H
#define WIN16_CONTROL_FRAME_STATE_H
#include "../win16-common/palette.h"
#include "menu_scope.h"
#include "menu_items.h"
typedef struct menu_loop {
    win16_style_menu_scope *scope;
    unsigned depth;
    struct menu_loop *previous;
} menu_loop;
typedef struct style_window {
    win16_style_palette palette;
    HFONT title_font,menu_font; /* Title unchanged; menu owns a System variant. */
    UINT dpi, calls;
    BOOL active, detached, hot, system_menu, dialog, menu_only;
    int pressed,bar_selected,bar_direction,bar_hover;
    BOOL bar_keyboard;
    HWND bar_focus;
    win16_style_menu_item *menu_items;
    menu_loop *menus;
    unsigned menu_depth;
    DWORD old_policy;
    BOOL policy_saved, pending_frame, xaml_repainting, xaml_repaint_pending, xaml_waiting, keep_outer, xaml_icon_update;
    HWND xaml_repaint_window, xaml_repaint_owner;
    ULONGLONG xaml_deadline;
    void (*xaml_activate)(HWND,struct style_window *,const RECT *,BOOL);
    const void *creation_owner;
} style_window;

style_window *win16_control_frame_state_create(void);
void win16_control_frame_state_enter_menu(style_window *state);
void win16_control_frame_state_exit_menu(style_window *state);
void win16_control_frame_state_end_menu(style_window *state);
void win16_control_frame_state_dispose(style_window *state);
#endif
