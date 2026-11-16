#ifndef WIN16_CONTROL_OTHER_PAINT_H
#define WIN16_CONTROL_OTHER_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/lifecycle/instance.h"
void win16_control_other(HWND window,style_instance *instance,const win16_style_palette *palette,
    HDC dc,RECT bounds,HFONT font,const WCHAR *text,BOOL label,
    UINT (*button_type)(HWND,DWORD),BOOL (*group_push)(HWND));
#endif
