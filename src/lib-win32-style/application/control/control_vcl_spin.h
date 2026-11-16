#ifndef STYLE_CONTROL_VCL_SPIN_H
#define STYLE_CONTROL_VCL_SPIN_H
#include <windows.h>
HWND style_control_vcl_spinedit_button(HWND window);
HWND style_control_vcl_spin_before(HWND window,UINT message,RECT *before);
void style_control_vcl_spin_after(HWND window,HWND button,const RECT *before,const BOOL *detached);
BOOL style_control_vcl_spin_identity(HWND window);
BOOL style_control_vcl_modern_spin_shape(HWND window);
#endif
