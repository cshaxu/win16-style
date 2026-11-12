#include "lib-win32/style/window_hook.h"
#include <stdio.h>

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
    RemovePropW(window,property);DestroyWindow(window);
    if(!ok)return 3;
    puts("WINDOW_HOOK_OK reject=1 busy=1 result=1 cleanup=1");return 0;
}
