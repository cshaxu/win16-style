#ifndef STYLE_CONTROL_OUTLINE_COMBO_H
#define STYLE_CONTROL_OUTLINE_COMBO_H
#include "../window/palette.h"
#include "control_colors.h"
#include "lib-win32-style/application/control/control_state.h"
#include "lib-win32-style/application/control/control_native_combo.h"
typedef struct style_outline_combo_view {
    style_instance *instance;
    const style_control_state *state;
    const style_window_palette *palette;
    const classic_combo_colors *button;
    const classic_list_colors *list;
    const classic_text_colors *text;
    const classic_combo_arrow_art *arrow;
    BOOL dialog_combo;
} style_outline_combo_view;
void style_outline_draw_combo(HWND,const style_outline_combo_view *,HDC,BOOL (*)(HWND,PCOMBOBOXINFO),
    BOOL (*)(HWND),BOOL (*)(HWND));
void style_outline_native_combo_button(HWND window,const style_instance *instance,HDC dc,const RECT *dirty,const style_control_native_button *button,const style_window_palette *,const classic_combo_arrow_art *);
#endif
