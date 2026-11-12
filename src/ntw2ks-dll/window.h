#ifndef NTW2KS_WINDOW_H
#define NTW2KS_WINDOW_H
#include <windows.h>

/* The caller retains this module and invokes these on the window's GUI thread. */
BOOL __cdecl Ntw2ksWindowAttach(HWND window);
BOOL __cdecl Ntw2ksWindowAttached(HWND window);
BOOL __cdecl Ntw2ksWindowRemove(HWND window);
#endif
