#include "../lib-win32/win16-control/vb_text.h"
BOOL ntw16s_vb_user_label_target(HWND);
void ntw16s_vb_paint_begin(ntw16s_vb_paint *scope,HWND window,UINT message,ntw16s_vb_fonts *fonts)
{win16_control_vb_paint_begin(scope,window,message,fonts,ntw16s_vb_user_label_target);}
