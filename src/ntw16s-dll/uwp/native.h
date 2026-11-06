#ifndef NTW16S_UWP_NATIVE_H
#define NTW16S_UWP_NATIVE_H
#include <windows.h>

HWND ntw16s_uwp_title(HWND frame,HWND *auxiliary);
/* Caller initializes COM and must not be the target GUI thread. S_FALSE means
 * the native title does not offer a full-screen invocation. */
HRESULT ntw16s_uwp_fullscreen_available(HWND title);
/* GUI-thread menu callback. Native UIA work owns an independent MTA thread and
 * module reference; no GUI-thread wait and no retained COM provider cache. */
void ntw16s_uwp_fullscreen(HWND frame,void *title);
#endif
