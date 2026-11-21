#include "window.h"
#include "lib-win32-style/appearance/win2000/window/frame.h"

#include "ntwshk-dll/observed.h"
#ifdef NTWSHK_UNIFIED
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/window/window_identity.h"
#endif
static BOOL child(HWND window){return (GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)!=0;}
BOOL __cdecl Ntw2ksWindowAttach(HWND window)
{
#ifdef NTWSHK_UNIFIED
    if(!win32_style_window_current_thread(window)){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    if(!classic_theme_require_layout(CLASSIC_WINDOW_PROVIDER_NONCLIENT))return FALSE;
#endif
    return child(window)?ntwshk_observed_control_attach(window):win2k_frame_attach(window);
}
BOOL __cdecl Ntw2ksWindowAttached(HWND window){return ntwshk_observed_control_attached(window) || (!child(window) && win2k_frame_attached(window));}
BOOL __cdecl Ntw2ksWindowRemove(HWND window){return child(window) || ntwshk_observed_control_attached(window)?ntwshk_observed_control_remove(window):win2k_frame_remove(window);}
