#include "window_target.h"
#include "ntw16s-dll/window_thread.h"
#include "lib-win32-style/access/runtime/module.h"
#include "lib-win32-style/access/runtime/window_dispatch.h"
#include <stdint.h>
#include <wchar.h>

DWORD run16s_window_module_open(DWORD pid,PCWSTR directory,HMODULE *module,HOOKPROC *callback)
{
    WCHAR path[32768];size_t length=wcslen(directory);
    if(length+14>=ARRAYSIZE(path))return ERROR_FILENAME_EXCED_RANGE;
    wcscpy(path,directory);wcscat(path,L"ntwshk64.dll");
    return win32_style_module_open(pid,path,"Ntw16sWindowThreadHook",module,callback);
}

DWORD run16s_window_send(HWND window,UINT message)
{return win32_style_window_request(window,message,NTW16S_WINDOW_THREAD_RESULT,
    NTW16S_WINDOW_THREAD_ATTACH,(LPARAM)window,3000);}

DWORD run16s_window_attach(HWND window,DWORD expected_pid,DWORD expected_thread,PCWSTR directory)
{
    HMODULE module;HOOKPROC callback;
    DWORD error=run16s_window_module_open(expected_pid,directory,&module,&callback);
    if(error)return error;
    error=win32_style_window_dispatch_loaded(window,expected_pid,expected_thread,module,callback,
        NTW16S_WINDOW_THREAD_MESSAGE,NTW16S_WINDOW_THREAD_RESULT,NTW16S_WINDOW_THREAD_ATTACH,(LPARAM)window,3000);
    FreeLibrary(module);return error;
}
