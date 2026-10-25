#ifndef NTW16S_TRACKBAR_PAINT_H
#define NTW16S_TRACKBAR_PAINT_H
#include "win16_style.h"
#include <commctrl.h>
BOOL win16_style_trackbar_supported(HWND);
BOOL win16_style_trackbar_part(HWND,const NMCUSTOMDRAW *,const win16_style_palette *);
#endif
