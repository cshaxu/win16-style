#include "window_dispatch.h"
#include <stdint.h>

DWORD win32_style_window_request(HWND window,UINT message,PCWSTR property,
    WPARAM action,LPARAM parameter,DWORD timeout)
{
    DWORD_PTR ignored=0;DWORD error;
    RemovePropW(window,property);
    if(message)SetLastError(ERROR_SUCCESS);
    if(!message || !SendMessageTimeoutW(window,message,action,parameter,
        SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,timeout,&ignored))error=GetLastError()?GetLastError():ERROR_TIMEOUT;
    else {uintptr_t result=(uintptr_t)GetPropW(window,property);error=result?(DWORD)(result-1):ERROR_INVALID_DATA;}
    RemovePropW(window,property);return error;
}
DWORD win32_style_window_dispatch_loaded(HWND window,DWORD expected_pid,DWORD thread,
    HMODULE module,HOOKPROC callback,PCWSTR message_name,PCWSTR property,
    WPARAM action,LPARAM parameter,DWORD timeout)
{
    if(!window || !expected_pid || !thread)return ERROR_INVALID_WINDOW_HANDLE;
    HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,module,thread);
    DWORD error=hook?ERROR_SUCCESS:GetLastError();
    if(!hook)return error?error:ERROR_PROC_NOT_FOUND;
    DWORD pid=0,current=GetWindowThreadProcessId(window,&pid);
    UINT message=RegisterWindowMessageW(message_name);
    if(pid!=expected_pid || current!=thread || !message)error=ERROR_INVALID_WINDOW_HANDLE;
    else error=win32_style_window_request(window,message,property,action,parameter,timeout);
    if(!UnhookWindowsHookEx(hook) && !error)error=GetLastError();
    return error;
}
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
    DWORD error=win32_style_window_dispatch_loaded(window,process,thread,hook_module,callback,
        message_name,result_property,action,parameter,timeout);
    FreeLibrary(module);return error;
}
