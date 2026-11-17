#ifndef WIN2K_CONTROL_BACKGROUND_H
#define WIN2K_CONTROL_BACKGROUND_H
#include "../common/palette.h"
#include "lib-win32-style/application/lifecycle/instance.h"
/* Caller saves/restores the DC and pins instance. Query once, preserve app
 * colors/mode and align its borrowed brush to parent coordinates. The stock
 * system face maps to the profile DC_BRUSH; no application brush is deleted. */
HBRUSH win2k_control_brush(HWND,style_instance *,HDC,const win2k_palette *);
#endif
