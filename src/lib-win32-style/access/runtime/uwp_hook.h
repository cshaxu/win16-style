#ifndef STYLE_UWP_HOOK_H
#define STYLE_UWP_HOOK_H
#include <windows.h>
typedef struct style_uwp_hook_spec {
    PCWSTR message,result_property;
    BOOL (*accept)(WPARAM,void *);
    DWORD (*perform)(HWND,HWND,WPARAM,void *);
    void *context;
} style_uwp_hook_spec;
/* Called by a caller-owned thread hook. busy is caller-owned thread-local
 * state; the caller supplies protocol, action policy and style behavior.
 * This handles the message only; the caller still calls CallNextHookEx. */
void style_uwp_hook_handle(int,LPARAM,BOOL *,const style_uwp_hook_spec *);
#endif
