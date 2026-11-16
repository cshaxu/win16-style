#include "module.h"
#include <psapi.h>
#include <tlhelp32.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

/* SetWindowsHookEx uses the DLL's code offset in the target. If an older DLL
 * is already mapped there, the same offset need not be the same function. */
DWORD win32_style_module_compatible(DWORD pid,HMODULE module,HOOKPROC callback,PCWSTR path)
{
    PCWSTR name=wcsrchr(path,L'\\');
    if(!name || !name[1])return ERROR_INVALID_PARAMETER;
    ++name;
    HANDLE list=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
    if(list==INVALID_HANDLE_VALUE)return GetLastError();
    MODULEENTRY32W entry={.dwSize=sizeof(entry)};DWORD error=0;BOOL found=FALSE;
    if(!Module32FirstW(list,&entry)){error=GetLastError();CloseHandle(list);return error;}
    do {
        if(_wcsicmp(entry.szModule,name))continue;
        found=TRUE;
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
        if(!error){
            if(offset+sizeof(remote)>entry.modBaseSize)error=ERROR_REVISION_MISMATCH;
            else if(!ReadProcessMemory(process,entry.modBaseAddr+offset,remote,sizeof(remote),&received)){
                error=GetLastError();if(!error)error=ERROR_PARTIAL_COPY;
            }else if(received!=sizeof(remote))error=ERROR_PARTIAL_COPY;
            else if(memcmp(remote,(const void *)(uintptr_t)callback,sizeof(remote)))error=ERROR_REVISION_MISMATCH;
        }
        CloseHandle(process);break;
    }while(Module32NextW(list,&entry));
    if(!found){error=GetLastError();if(error==ERROR_NO_MORE_FILES)error=ERROR_SUCCESS;else if(!error)error=ERROR_GEN_FAILURE;}
    CloseHandle(list);return error;
}


DWORD win32_style_module_open(DWORD pid,PCWSTR path,const char *name,HMODULE *module,HOOKPROC *callback)
{
    HMODULE loaded=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!loaded)return GetLastError();
    HOOKPROC entry=(HOOKPROC)(uintptr_t)GetProcAddress(loaded,name);
    DWORD error=entry?win32_style_module_compatible(pid,loaded,entry,path):ERROR_PROC_NOT_FOUND;
    if(error){FreeLibrary(loaded);return error;}
    *module=loaded;*callback=entry;return ERROR_SUCCESS;
}
