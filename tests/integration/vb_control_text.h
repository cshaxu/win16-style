#ifndef WIN16STYLE_VB_CONTROL_TEXT_H
#define WIN16STYLE_VB_CONTROL_TEXT_H
#include <windows.h>

/* WM_GETTEXT is marshalled by USER across processes; GetWindowTextW returns
 * cached window text instead of querying another process's control. */
static BOOL vb_control_text(HWND window,WCHAR *text,UINT capacity)
{
    DWORD_PTR length=0;
    if(!text || !capacity){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    text[0]=L'\0';
    return SendMessageTimeoutW(window,WM_GETTEXT,capacity,(LPARAM)text,
        SMTO_ABORTIFHUNG|SMTO_BLOCK,1000,&length)!=0;
}
#endif
