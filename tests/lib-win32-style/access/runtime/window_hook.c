#include "lib-win32-style/access/runtime/window_hook.h"
#include <stdio.h>

typedef struct operation_context {BOOL busy,reentered,destroy;unsigned calls;const style_window_hook_spec *spec;} operation_context;
static BOOL accepts(const CWPSTRUCT *message,void *context)
{(void)context;return message->wParam==1;}
static DWORD perform(const CWPSTRUCT *message,void *context)
{
    operation_context *state=context;++state->calls;
    style_window_hook_handle(0,(LPARAM)message,&state->busy,state->spec);
    state->reentered=state->busy && state->calls==1;
    if(state->destroy)DestroyWindow(message->hwnd);
    return ERROR_ACCESS_DENIED;
}
int main(void)
{
    HWND window=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,30,30,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window)return 1;
    PCWSTR name=L"Style.WindowHook.Test",property=L"Style.WindowHook.Result";
    CWPSTRUCT message={.hwnd=window,.message=RegisterWindowMessageW(name),.wParam=1};
    if(win32_style_window_hook_match(-1,(LPARAM)&message,FALSE,name) ||
       win32_style_window_hook_match(0,(LPARAM)&message,TRUE,name) ||
       win32_style_window_hook_match(0,0,FALSE,name) ||
       win32_style_window_hook_match(0,(LPARAM)&message,FALSE,L"Style.WindowHook.Other") ||
       win32_style_window_hook_match(0,(LPARAM)&message,FALSE,name)!=&message)return 2;
    win32_style_window_hook_finish(window,property,ERROR_ACCESS_DENIED);
    BOOL ok=(uintptr_t)GetPropW(window,property)==ERROR_ACCESS_DENIED+1;
    RemovePropW(window,property);
    operation_context state={0};
    const style_window_hook_spec spec={name,property,accepts,perform,&state};state.spec=&spec;
    message.wParam=2;style_window_hook_handle(0,(LPARAM)&message,&state.busy,&spec);
    if(state.calls || GetPropW(window,property))return 4;
    message.wParam=1;style_window_hook_handle(0,(LPARAM)&message,&state.busy,&spec);
    if(state.calls!=1 || !state.reentered || state.busy || (uintptr_t)GetPropW(window,property)!=ERROR_ACCESS_DENIED+1)return 5;
    RemovePropW(window,property);state.calls=0;state.destroy=TRUE;
    style_window_hook_handle(0,(LPARAM)&message,&state.busy,&spec);
    if(state.calls!=1 || state.busy || IsWindow(window))return 6;

    if(!ok)return 3;
    puts("WINDOW_HOOK_OK reject=1 busy=1 result=1 cleanup=1 recursive=1 destroyed=1");return 0;
}
