#ifndef WIN16_UWP_FRAME_H
#define WIN16_UWP_FRAME_H
#include <windows.h>

/* Local GUI-thread API. The caller retains the containing module until every
 * frame has been removed and callbacks have returned. No process hooks, XAML
 * changes, target file writes or registry writes occur here. */
BOOL win16_uwp_frame_apply(HWND frame,HWND native_title);
BOOL win16_uwp_frame_remove(HWND frame);
HWND win16_uwp_frame_surface(HWND frame);
#endif
