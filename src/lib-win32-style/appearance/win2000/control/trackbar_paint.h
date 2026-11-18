#ifndef WIN2K_TRACKBAR_PAINT_H
#define WIN2K_TRACKBAR_PAINT_H
#include <windows.h>
#include <commctrl.h>

BOOL win2k_trackbar_supported(HWND);
BOOL win2k_trackbar_part(HWND,const NMCUSTOMDRAW *,const void *);
#endif
