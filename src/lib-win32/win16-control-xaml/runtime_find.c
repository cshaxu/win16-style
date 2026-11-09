#include "runtime_find.h"
#include <tlhelp32.h>
#include <wchar.h>

DWORD win16_xaml_runtime_find(HANDLE process, WCHAR *loader, DWORD loader_capacity,
                             WCHAR *xaml, DWORD xaml_capacity)
{
    typedef DWORD (WINAPI *module_path_fn)(HANDLE,HMODULE,LPWSTR,DWORD);
    if (!process || !loader || !loader_capacity || !xaml || !xaml_capacity) return ERROR_INVALID_PARAMETER;
    *loader=0; *xaml=0;
    DWORD pid=GetProcessId(process);
    if (!pid) return GetLastError();
    module_path_fn module_path=(module_path_fn)(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"K32GetModuleFileNameExW");
    if (!module_path) return ERROR_PROC_NOT_FOUND;
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
    if (snapshot==INVALID_HANDLE_VALUE) return GetLastError();
    MODULEENTRY32W item={.dwSize=sizeof(item)};
    DWORD error=ERROR_SUCCESS;
    if (!Module32FirstW(snapshot,&item)) error=GetLastError();
    else do {
        WCHAR *out=NULL; DWORD capacity=0;
        if (!_wcsicmp(item.szModule,L"Microsoft.Internal.FrameworkUdk.dll")) { out=loader; capacity=loader_capacity; }
        else if (!_wcsicmp(item.szModule,L"Microsoft.UI.Xaml.dll")) { out=xaml; capacity=xaml_capacity; }
        if (out) {
            if (*out) { error=ERROR_DUP_NAME; break; }
            DWORD length=module_path(process,item.hModule,out,capacity);
            if (!length || length>=capacity) { error=length ? ERROR_INSUFFICIENT_BUFFER : GetLastError(); break; }
        }
    } while (Module32NextW(snapshot,&item));
    if (!error && GetLastError()!=ERROR_NO_MORE_FILES) error=GetLastError();
    if (!error && (!*loader || !*xaml)) error=ERROR_MOD_NOT_FOUND;
    CloseHandle(snapshot);
    if (error) { *loader=0; *xaml=0; }
    return error;
}
