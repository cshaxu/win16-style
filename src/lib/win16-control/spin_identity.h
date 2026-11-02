#ifndef WIN16_CONTROL_SPIN_IDENTITY_H
#define WIN16_CONTROL_SPIN_IDENTITY_H
#include <windows.h>
HWND win16_control_spinedit_button(HWND window);
BOOL win16_control_spin_identity(HWND window);
BOOL win16_control_modern_spin_shape(HWND window);
BOOL win16_control_spin_full_clip(HDC dc,const POINT points[2]);
#endif
