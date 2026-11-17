#ifndef WIN2K_BUTTON_PAINT_H
#define WIN2K_BUTTON_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/lifecycle/instance.h"

BOOL win2k_push_button_frame(HDC,RECT *,UINT state,BOOL default_button,const win2k_control_palette *);
/* Synchronous native BUTTON snapshot. Caller pins instance and owns the DC;
 * application font/images are borrowed and its input protocol stays native. */
BOOL win2k_button_paint(HWND,style_instance *,HDC,const win2k_control_palette *);
#endif
