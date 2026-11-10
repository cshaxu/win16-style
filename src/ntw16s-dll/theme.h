#ifndef NTW16S_THEME_H
#define NTW16S_THEME_H
#include "api.h"
#include "../lib-win32/win16-control/theme_state.h"
/* Private same-thread theme ownership for newly created controls. Capture
 * keeps native rendering unchanged; enable starts the local override. */
BOOL ntw16s_theme_enable(HWND,const void *owner);
void ntw16s_theme_remove(HWND,const void *owner);
NTW16S_API HRESULT __cdecl Ntw16sSetWindowTheme(HWND,LPCWSTR,LPCWSTR);
#endif
