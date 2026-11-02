#include "theme.h"
#include "native_api.h"
#include "../lib/win16-control/theme_state.h"
BOOL ntw16s_theme_capture(HWND w){return win16_control_theme_capture(w);}
BOOL ntw16s_theme_stage(HWND w){return win16_control_theme_stage(w);}
BOOL ntw16s_theme_commit(HWND w){return win16_control_theme_commit(w);}
void ntw16s_theme_cancel_stage(HWND w){win16_control_theme_cancel_stage(w);}
BOOL ntw16s_theme_terminal(HWND w,const void *owner){return win16_control_theme_terminal(w,owner);}
BOOL ntw16s_theme_enable(HWND w,const void *owner){return win16_control_theme_enable(w,owner,ntw16s_native_set_window_theme);}
BOOL ntw16s_theme_retained(HWND w){return win16_control_theme_retained(w);}
void ntw16s_theme_remove(HWND w,const void *owner){win16_control_theme_remove(w,owner,ntw16s_native_set_window_theme);}
HRESULT __cdecl Ntw16sSetWindowTheme(HWND w,LPCWSTR application,LPCWSTR ids)
{return win16_control_theme_set(w,application,ids,ntw16s_native_set_window_theme);}
