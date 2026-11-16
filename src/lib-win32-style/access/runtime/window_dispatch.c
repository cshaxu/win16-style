#include "window_dispatch.h"
#include <stdint.h>

DWORD win32_style_window_dispatch(HWND window,DWORD thread,PCWSTR module_path,
    const char *hook_export,PCWSTR message_name,PCWSTR result_property,
    WPARAM action,LPARAM parameter,DWORD timeout)
{
    HMODULE module=LoadLibraryExW(module_path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!module)return GetLastError();
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(module,hook_export);
    if(!callback){DWORD error=GetLastError();FreeLibrary(module);return error;}
    DWORD process=0,window_thread=GetWindowThreadProcessId(window,&process);
    HMODULE hook_module=window_thread==thread && process==GetCurrentProcessId()?NULL:module;
    HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,hook_module,thread);
    DWORD error=hook?0:GetLastError();
    if(hook){
        RemovePropW(window,result_property);DWORD_PTR ignored=0;
        UINT message=RegisterWindowMessageW(message_name);
        if(message)SetLastError(ERROR_SUCCESS);
        if(!SendMessageTimeoutW(window,message,action,parameter,
            SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,timeout,&ignored))error=GetLastError()?GetLastError():ERROR_TIMEOUT;
        else {uintptr_t result=(uintptr_t)GetPropW(window,result_property);error=result?(DWORD)(result-1):ERROR_INVALID_DATA;}
        RemovePropW(window,result_property);
        if(!UnhookWindowsHookEx(hook)&&!error)error=GetLastError();
    }
    FreeLibrary(module);return error;
}
