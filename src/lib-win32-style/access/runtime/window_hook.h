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

typedef struct style_window_hook_spec {
    PCWSTR message,result_property;
    BOOL (*accept)(const CWPSTRUCT *,void *);
    DWORD (*perform)(const CWPSTRUCT *,void *);
    void *context;
} style_window_hook_spec;
/* busy is caller-owned thread-local state. The action must return normally;
 * the caller retains CallNextHookEx and its original LastError contract. */
static inline void style_window_hook_handle(int code,LPARAM parameter,BOOL *busy,
    const style_window_hook_spec *spec)
{
    if(code<0 || *busy)return;
    const CWPSTRUCT *message=win32_style_window_hook_match(code,parameter,*busy,spec->message);
    if(!message || !spec->accept(message,spec->context))return;
    *busy=TRUE;
    DWORD error=spec->perform(message,spec->context);
    win32_style_window_hook_finish(message->hwnd,spec->result_property,error);
    *busy=FALSE;
}
#endif
