#ifndef NTW16S_STATUS_THEME_H
#define NTW16S_STATUS_THEME_H
#include <windows.h>
typedef struct ntw16s_status_theme_scope {struct ntw16s_status_theme_scope *previous;void *paint;HDC dc;int length;BOOL valid;WCHAR text[513];} ntw16s_status_theme_scope;
BOOL ntw16s_status_theme_begin(ntw16s_status_theme_scope *,HDC,int,int,LPCWSTR,int,DWORD,const RECT *,const void *);
void ntw16s_status_theme_end(void *);
#endif
