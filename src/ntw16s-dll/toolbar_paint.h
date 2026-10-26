#ifndef NTW16S_TOOLBAR_PAINT_H
#define NTW16S_TOOLBAR_PAINT_H
#include "win16_style.h"
#include <commctrl.h>
BOOL win16_style_toolbar_supported(HWND);
BOOL win16_style_toolbar_part(HWND,const NMCUSTOMDRAW *,const win16_style_palette *);
#endif
