#include "remote_loader.h"
#include "lib-win32/process-image/remote.h"
#include "module_snapshot.h"
#include "remote_memory.h"
#include <stdint.h>
#include <string.h>
#include <wchar.h>

DWORD win32_process_global_remote_loadlibrary(HANDLE process,DWORD pid,void **address)
{
    HMODULE kernel=GetModuleHandleW(L"kernel32.dll"),local=NULL;FARPROC function;
    WCHAR path[MAX_PATH],system[MAX_PATH],wow[MAX_PATH];DWORD length,system_length,wow_length,error=ERROR_SUCCESS;
    HANDLE snapshot=INVALID_HANDLE_VALUE;MODULEENTRY32W entry={.dwSize=sizeof(entry)};
    IMAGE_NT_HEADERS local_header,remote_header;BYTE local_bytes[16],remote_bytes[16];
    uintptr_t offset;void *found=NULL;
    if(address)*address=NULL;
    if(!process || !pid || !address || GetProcessId(process)!=pid)return ERROR_INVALID_PARAMETER;
    function=kernel?GetProcAddress(kernel,"LoadLibraryW"):NULL;
    if(!function || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          (LPCWSTR)(void *)function,&local))return ERROR_PROC_NOT_FOUND;
    length=GetModuleFileNameW(local,path,ARRAYSIZE(path));
    if(!length){error=GetLastError();return error?error:ERROR_INVALID_DATA;}
    if(length>=ARRAYSIZE(path))return ERROR_INSUFFICIENT_BUFFER;
    system_length=GetSystemDirectoryW(system,ARRAYSIZE(system));
    if(!system_length){error=GetLastError();return error?error:ERROR_INVALID_DATA;}
    if(system_length>=ARRAYSIZE(system))return ERROR_INSUFFICIENT_BUFFER;
    wow_length=GetSystemWow64DirectoryW(wow,ARRAYSIZE(wow));
    const WCHAR *base=wcsrchr(path,L'\\');
    if(!base || (_wcsicmp(base+1,L"kernelbase.dll") && _wcsicmp(base+1,L"kernel32.dll")) ||
       !((!_wcsnicmp(path,system,system_length) && path[system_length]==L'\\' && base==path+system_length) ||
         (wow_length && wow_length<ARRAYSIZE(wow) && !_wcsnicmp(path,wow,wow_length) && path[wow_length]==L'\\' && base==path+wow_length)))return ERROR_ACCESS_DENIED;
    error=win32_process_image_remote_header(GetCurrentProcess(),local,&local_header);if(error)return error;
    offset=(uintptr_t)(void *)function-(uintptr_t)local;
    if(offset>local_header.OptionalHeader.SizeOfImage ||
       sizeof(local_bytes)>local_header.OptionalHeader.SizeOfImage-offset)return ERROR_BAD_EXE_FORMAT;
    if(!win32_process_image_remote_executable(GetCurrentProcess(),(void *)function,local))return ERROR_INVALID_ADDRESS;
    error=win32_process_global_remote_read(GetCurrentProcess(),(void *)function,local_bytes,sizeof(local_bytes));if(error)return error;
    snapshot=win32_process_global_module_snapshot(pid);
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    if(!Module32FirstW(snapshot,&entry)){error=GetLastError();goto done;}
    do{
        if(_wcsicmp(entry.szExePath,path))continue;
        if(found){error=ERROR_DUP_NAME;goto done;}
        error=win32_process_image_remote_header(process,entry.modBaseAddr,&remote_header);if(error)goto done;
        if(remote_header.OptionalHeader.SizeOfImage!=local_header.OptionalHeader.SizeOfImage ||
           remote_header.FileHeader.TimeDateStamp!=local_header.FileHeader.TimeDateStamp ||
           entry.modBaseSize!=local_header.OptionalHeader.SizeOfImage){error=ERROR_REVISION_MISMATCH;goto done;}
        if((uintptr_t)entry.modBaseAddr>UINTPTR_MAX-offset){error=ERROR_INVALID_ADDRESS;goto done;}
        found=entry.modBaseAddr+offset;
        if(!win32_process_image_remote_executable(process,found,entry.modBaseAddr)){error=ERROR_INVALID_ADDRESS;goto done;}
        error=win32_process_global_remote_read(process,found,remote_bytes,sizeof(remote_bytes));if(error)goto done;
        if(memcmp(local_bytes,remote_bytes,sizeof(local_bytes))){error=ERROR_REVISION_MISMATCH;goto done;}
    }while(Module32NextW(snapshot,&entry));
    if(GetLastError()!=ERROR_NO_MORE_FILES){error=GetLastError();goto done;}
    if(!found){error=ERROR_MOD_NOT_FOUND;goto done;}
    *address=found;
done:
    CloseHandle(snapshot);return error;
}

