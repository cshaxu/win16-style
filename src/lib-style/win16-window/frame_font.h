#ifndef WIN16_CONTROL_FRAME_FONT_H
#define WIN16_CONTROL_FRAME_FONT_H
#include <windows.h>
/* Replace the owned title/menu leases together; leave them intact on failure. */
BOOL win16_control_refresh_frame_fonts(HWND window,UINT *dpi,HFONT *title,HFONT *menu);
#endif
