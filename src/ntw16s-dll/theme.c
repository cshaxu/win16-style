#include "theme.h"
#include "native_api.h"
BOOL ntw16s_theme_enable(HWND w,const void *owner){return style_theme_suppress(w,owner,ntw16s_native_set_window_theme);}
void ntw16s_theme_remove(HWND w,const void *owner){style_theme_remove(w,owner,ntw16s_native_set_window_theme);}
HRESULT __cdecl Ntw16sSetWindowTheme(HWND w,LPCWSTR application,LPCWSTR ids)
{return style_theme_set(w,application,ids,ntw16s_native_set_window_theme);}
