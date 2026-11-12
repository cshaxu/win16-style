#include "window.h"
#include "lib-win32/win2k-window/frame.h"

BOOL __cdecl Ntw2ksWindowAttach(HWND window){return win2k_frame_attach(window);}
BOOL __cdecl Ntw2ksWindowAttached(HWND window){return win2k_frame_attached(window);}
BOOL __cdecl Ntw2ksWindowRemove(HWND window){return win2k_frame_remove(window);}
