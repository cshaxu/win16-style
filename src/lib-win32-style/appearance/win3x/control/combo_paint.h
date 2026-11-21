#ifndef WIN16_CONTROL_COMBO_PAINT_H
#define WIN16_CONTROL_COMBO_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/control_state.h"
#include "lib-win32-style/application/control/control_native_combo.h"
struct control_style;
void win16_control_draw_combo(HWND window,struct control_style *style,HDC dc,BOOL (*get_info)(HWND,PCOMBOBOXINFO),
    BOOL (*is_vb_combo)(HWND),BOOL (*is_vb_drive)(HWND));
void win16_control_native_combo_button(HWND window,const style_instance *instance,HDC dc,const RECT *dirty,const style_control_native_button *button,const win16_style_palette *,const classic_combo_arrow_art *);
#endif
