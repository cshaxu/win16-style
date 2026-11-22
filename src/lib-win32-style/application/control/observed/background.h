#ifndef STYLE_OBSERVED_CONTROL_BACKGROUND_H
#define STYLE_OBSERVED_CONTROL_BACKGROUND_H
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/application/lifecycle/instance.h"
/* Caller saves/restores the DC and pins instance. Query once, preserve app
 * colors/mode and align its borrowed brush to parent coordinates. The stock
 * system face maps to the profile DC_BRUSH; no application brush is deleted. */
HBRUSH style_observed_control_brush(HWND,style_instance *,HDC,const style_relief_palette *);
HBRUSH style_observed_control_color_brush(HWND window,HWND parent,style_instance *,HDC,UINT message,
    const style_relief_palette *,COLORREF background);
#endif
