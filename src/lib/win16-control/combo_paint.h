#ifndef WIN16_CONTROL_COMBO_PAINT_H
#define WIN16_CONTROL_COMBO_PAINT_H
#include "palette.h"
void win16_control_draw_combo(HWND window,const BOOL *detached,const BOOL *common_dialog_combo,
    const win16_style_palette *palette,HFONT *font,HDC dc,BOOL (*get_info)(HWND,PCOMBOBOXINFO),
    BOOL (*is_vb_combo)(HWND),BOOL (*is_vb_drive)(HWND));
void win16_control_native_combo_button(HWND window,const BOOL *detached,HDC dc,const RECT *dirty,RECT bounds,BOOL pressed);
#endif
