#ifndef STYLE_UWP_NATIVE_H
#define STYLE_UWP_NATIVE_H
#include <windows.h>

/* auxiliary is required; returned handles are borrowed from the host. */
HWND style_uwp_title(HWND frame,HWND *auxiliary);
/* Caller initializes COM and must not be the target GUI thread. S_FALSE means
 * the native title does not offer a full-screen invocation. */
HRESULT style_uwp_fullscreen_available(HWND title);
/* GUI-thread menu callback. Native UIA work owns an independent MTA thread and
 * module reference; no GUI-thread wait and no retained COM provider cache. */
void style_uwp_fullscreen(HWND frame,void *title);
#endif
