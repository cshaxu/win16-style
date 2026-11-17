#ifndef NTW16S_PROGRESS_PAINT_H
#define NTW16S_PROGRESS_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/native.h"
BOOL win16_style_progress_supported(HWND);
BOOL win16_style_progress_paint(HWND,HDC,const win16_style_palette *,const style_instance *,const style_control_native_state *);
#endif
