#ifndef WIN2K_WINDOW_FRAME_H
#define WIN2K_WINDOW_FRAME_H
#include <windows.h>

/* Install on the window's owning GUI thread. */
BOOL win2k_frame_attach(HWND window);
#endif
