#include "window_target.h"
#include "ntw16s-dll/window_thread.h"
#include <psapi.h>
#include <stdint.h>
#include <string.h>
#include <tlhelp32.h>
#include <wchar.h>

/* SetWindowsHookEx uses the DLL's code offset in the target. If an older DLL
 * is already mapped there, the same offset need not be the same function. */
DWORD run16s_window_module_compatible(DWORD pid,HMODULE module,HOOKPROC callback,PCWSTR path)
{
    HANDLE list=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
    if(list==INVALID_HANDLE_VALUE)return GetLastError();
    MODULEENTRY32W entry={.dwSize=sizeof(entry)};DWORD error=0;
    if(!Module32FirstW(list,&entry)){error=GetLastError();CloseHandle(list);return error;}
    do {
        if(_wcsicmp(entry.szModule,L"ntw16s64.dll"))continue;
        if(_wcsicmp(entry.szExePath,path)){error=ERROR_REVISION_MISMATCH;break;}
        HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,FALSE,pid);
        if(!process){error=GetLastError();break;}
        /* The loader's stored path does not change when a mapped DLL is
         * renamed. Compare the mapped files as well as the loader names. */
        WCHAR local_mapping[2048],remote_mapping[2048];
        DWORD local_length=GetMappedFileNameW(GetCurrentProcess(),module,local_mapping,ARRAYSIZE(local_mapping));
        if(!local_length)error=GetLastError()?GetLastError():ERROR_GEN_FAILURE;
        DWORD remote_length=error?0:GetMappedFileNameW(process,entry.modBaseAddr,remote_mapping,ARRAYSIZE(remote_mapping));
        if(!error && !remote_length)error=GetLastError()?GetLastError():ERROR_GEN_FAILURE;
        if(!error && (local_length>=ARRAYSIZE(local_mapping)-1 || remote_length>=ARRAYSIZE(remote_mapping)-1))
            error=ERROR_INSUFFICIENT_BUFFER;
        if(!error && _wcsicmp(local_mapping,remote_mapping))error=ERROR_REVISION_MISMATCH;
        BYTE remote[64];SIZE_T received=0;uintptr_t offset=(uintptr_t)callback-(uintptr_t)module;
        if(!error && (offset+sizeof(remote)>entry.modBaseSize ||
           !ReadProcessMemory(process,entry.modBaseAddr+offset,remote,sizeof(remote),&received) ||
           received!=sizeof(remote) || memcmp(remote,(const void *)(uintptr_t)callback,sizeof(remote))))
            error=ERROR_REVISION_MISMATCH;
        CloseHandle(process);break;
    }while(Module32NextW(list,&entry));
    CloseHandle(list);return error;
}


DWORD run16s_window_attach(HWND window,DWORD expected_pid,DWORD expected_thread,PCWSTR directory)
{
    WCHAR path[32768];size_t length=wcslen(directory);
    if(length+14>=ARRAYSIZE(path))return ERROR_FILENAME_EXCED_RANGE;
    wcscpy(path,directory);wcscat(path,L"ntw16s64.dll");
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!module)return GetLastError();
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(module,"Ntw16sWindowThreadHook");
    DWORD error=callback?run16s_window_module_compatible(expected_pid,module,callback,path):ERROR_PROC_NOT_FOUND;
    HHOOK hook=!error?SetWindowsHookExW(WH_CALLWNDPROC,callback,module,expected_thread):NULL;
    if(!hook && !error)error=GetLastError();
    if(!hook && !error)error=ERROR_PROC_NOT_FOUND;
    if(hook){
        DWORD pid=0,thread=GetWindowThreadProcessId(window,&pid);DWORD_PTR ignored=0;
        UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
        if(pid!=expected_pid || thread!=expected_thread || !message)error=ERROR_INVALID_WINDOW_HANDLE;
        else {
            RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
            if(!SendMessageTimeoutW(window,message,NTW16S_WINDOW_THREAD_ATTACH,(LPARAM)window,
                 SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,3000,&ignored))error=GetLastError()?GetLastError():ERROR_TIMEOUT;
            else {uintptr_t result=(uintptr_t)GetPropW(window,NTW16S_WINDOW_THREAD_RESULT);
                error=result?(DWORD)(result-1):ERROR_INVALID_DATA;}
            RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
        }
        if(!UnhookWindowsHookEx(hook) && !error)error=GetLastError();
    }
    FreeLibrary(module);return error;
}
