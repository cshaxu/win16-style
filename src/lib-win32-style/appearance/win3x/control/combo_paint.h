#ifndef WIN16_CONTROL_COMBO_PAINT_H
#define WIN16_CONTROL_COMBO_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/control_state.h"
#include "lib-win32-style/application/control/control_native_combo.h"
void win16_control_draw_combo(HWND window,style_instance *instance,const style_control_state *state,BOOL common_dialog_combo,
    const win16_style_palette *palette,HDC dc,BOOL (*get_info)(HWND,PCOMBOBOXINFO),
    BOOL (*is_vb_combo)(HWND),BOOL (*is_vb_drive)(HWND));
void win16_control_native_combo_button(HWND window,const style_instance *instance,HDC dc,const RECT *dirty,const style_control_native_button *button);
#endif
