#ifndef NTW16S_HEADER_PAINT_H
#define NTW16S_HEADER_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/native.h"
BOOL win16_style_header_supported(HWND);
struct classic_header_colors;
BOOL win16_style_header_paint(HWND,HDC,const win16_style_palette *,HFONT,const style_instance *,
    const style_control_native_state *,const struct classic_header_colors *);
#endif
