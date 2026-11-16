#ifndef WIN2K_WINDOW_FRAME_H
#define WIN2K_WINDOW_FRAME_H
#include <windows.h>

/* Install on the window's owning GUI thread. */
BOOL win2k_frame_attach(HWND window);
BOOL win2k_frame_attached(HWND window);
/* Remove a previously attached frame on the same GUI thread. */
BOOL win2k_frame_remove(HWND window);
#endif
