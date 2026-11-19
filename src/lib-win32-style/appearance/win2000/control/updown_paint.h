#ifndef WIN2K_UPDOWN_PAINT_H
#define WIN2K_UPDOWN_PAINT_H
#include "lib-win32-style/application/lifecycle/instance.h"
#include "../common/palette.h"

BOOL win2k_updown_supported(HWND);
BOOL win2k_updown_paint(HWND,style_instance *,HDC,const win2k_palette *);
#endif
