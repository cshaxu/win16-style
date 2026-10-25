#ifndef NTW16S_HEADER_PAINT_H
#define NTW16S_HEADER_PAINT_H
#include "win16_style.h"
BOOL win16_style_header_supported(HWND);
BOOL win16_style_header_changes(UINT);
BOOL win16_style_header_paint(HWND,HDC,const win16_style_palette *,HFONT,const BOOL *,const unsigned *,int);
#endif
