#ifndef NTW16S_STATUS_THEME_H
#define NTW16S_STATUS_THEME_H
#include <windows.h>
#include "lib-win32-style/application/control/status_paint.h"
BOOL ntw16s_status_theme_begin(style_control_status_theme *,HDC,int,int,LPCWSTR,int,DWORD,const RECT *,const void *);
void ntw16s_status_theme_end(void *);
#endif
