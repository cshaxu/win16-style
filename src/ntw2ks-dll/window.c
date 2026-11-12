#include "window.h"
#include "lib-style/win2k-window/frame.h"

BOOL ntw2ks_control_attach(HWND);
BOOL ntw2ks_control_attached(HWND);
BOOL ntw2ks_control_remove(HWND);
static BOOL child(HWND window){return (GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)!=0;}
BOOL __cdecl Ntw2ksWindowAttach(HWND window){return child(window)?ntw2ks_control_attach(window):win2k_frame_attach(window);}
BOOL __cdecl Ntw2ksWindowAttached(HWND window){return child(window)?ntw2ks_control_attached(window):win2k_frame_attached(window);}
BOOL __cdecl Ntw2ksWindowRemove(HWND window){return child(window)?ntw2ks_control_remove(window):win2k_frame_remove(window);}
