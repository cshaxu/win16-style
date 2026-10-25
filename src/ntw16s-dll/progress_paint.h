#ifndef NTW16S_PROGRESS_PAINT_H
#define NTW16S_PROGRESS_PAINT_H
#include "win16_style.h"
BOOL win16_style_progress_supported(HWND);
BOOL win16_style_progress_changes(UINT);
BOOL win16_style_progress_paint(HWND,HDC,const win16_style_palette *,const BOOL *,const unsigned *);
#endif
