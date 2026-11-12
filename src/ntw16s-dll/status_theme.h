#ifndef NTW16S_STATUS_THEME_H
#define NTW16S_STATUS_THEME_H
#include <windows.h>
#include "../lib-style/win16-control/status_text.h"
BOOL ntw16s_status_theme_begin(ntw16s_status_theme_scope *,HDC,int,int,LPCWSTR,int,DWORD,const RECT *,const void *);
void ntw16s_status_theme_end(void *);
#endif
