#ifndef WIN32_STYLE_WINDOW_HOOK_H
#define WIN32_STYLE_WINDOW_HOOK_H
#include <windows.h>
#include <stdint.h>

/* The caller owns reentrancy state, action policy and CallNextHookEx. */
static inline const CWPSTRUCT *win32_style_window_hook_match(int code,LPARAM parameter,
    BOOL busy,PCWSTR message_name)
{
    if(code<0 || busy)return NULL;
    const CWPSTRUCT *message=(const CWPSTRUCT *)parameter;
    return message && message->message==RegisterWindowMessageW(message_name)?message:NULL;
}
static inline void win32_style_window_hook_finish(HWND window,PCWSTR result_property,DWORD error)
{SetPropW(window,result_property,(HANDLE)(uintptr_t)((ULONGLONG)error+1));}
#endif
