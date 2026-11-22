#ifndef STYLE_CONTROL_OUTLINE_CHOICE_H
#define STYLE_CONTROL_OUTLINE_CHOICE_H
#include "../window/palette.h"
#include "control_colors.h"
#include "presentation.h"
#include "lib-win32-style/application/control/control_state.h"
typedef struct style_outline_choice_view {
    style_instance *instance;
    const style_control_state *state;
    const style_window_palette *palette;
    const classic_push_colors *push;
    const classic_choice_colors *choice;
    UINT (*button_type)(HWND,DWORD);
    BOOL (*is_static)(HWND);
    BOOL (*vb_frame)(HWND),(*vb_button)(HWND);
    BOOL (*group_choice)(HWND),(*group_push)(HWND);
    HMODULE module;
    classic_choice_art *art;
    style_choice_art_read_fn read_art;
    const classic_text_colors *label;
    const classic_group_colors *group;
    const style_frame_colors *frame;
    classic_control_presentation controls;
    BOOL choice_palette;
} style_outline_choice_view;
void style_outline_draw_choice(HWND window,const style_outline_choice_view *view,HDC dc);
#endif
