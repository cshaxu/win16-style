#ifndef STYLE_CONTROL_OUTLINE_BASIC_H
#define STYLE_CONTROL_OUTLINE_BASIC_H
#include "../window/palette.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include "lib-win32-style/application/lifecycle/instance.h"
void style_outline_draw_basic(HWND window,style_instance *instance,const style_window_palette *palette,
    HDC dc,RECT bounds,HFONT font,const WCHAR *text,BOOL label,
    UINT (*button_type)(HWND,DWORD),BOOL (*group_push)(HWND),classic_control_presentation controls,COLORREF text_color);
#endif
