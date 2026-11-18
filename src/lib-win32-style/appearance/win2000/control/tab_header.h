#ifndef WIN2K_TAB_HEADER_H
#define WIN2K_TAB_HEADER_H
#include "lib-win32-style/application/control/native.h"
#include <commctrl.h>

BOOL win2k_tab_supported(HWND);
BOOL win2k_header_supported(HWND);
BOOL win2k_tab_paint(HWND,HDC,HFONT,const style_instance *,const style_control_native_state *);
BOOL win2k_header_paint(HWND,const NMCUSTOMDRAW *,const void *);
#endif
