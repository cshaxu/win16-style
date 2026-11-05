#include "remote_export.h"
#include "lib-win32/process-image/remote.h"
#include <tlhelp32.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

DWORD win32_process_global_remote_export(HANDLE process,DWORD pid,const WCHAR *path,const WCHAR *expected_name,const char *target,void **address)
{
    typedef DWORD (WINAPI *module_path_fn)(HANDLE,HMODULE,LPWSTR,DWORD);
    module_path_fn module_path=(module_path_fn)(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"K32GetModuleFileNameExW");
    HANDLE snapshot=INVALID_HANDLE_VALUE;MODULEENTRY32W entry={.dwSize=sizeof(entry)};
    IMAGE_NT_HEADERS header;IMAGE_EXPORT_DIRECTORY exports;BYTE *base=NULL;DWORD error=ERROR_SUCCESS;
    WCHAR full[32752],canonical[32752],*filename=NULL;void *found=NULL;
    if(address)*address=NULL;
    if(!process || !pid || !path || !*path || !expected_name || !target || !*target || !address || GetProcessId(process)!=pid)return ERROR_INVALID_PARAMETER;
    size_t target_bytes=strlen(target)+1;if(target_bytes>256)return ERROR_INVALID_NAME;
    if(!module_path)return ERROR_PROC_NOT_FOUND;
    DWORD path_length=GetFullPathNameW(path,ARRAYSIZE(canonical),canonical,&filename);
    if(!path_length || path_length>=ARRAYSIZE(canonical) || !filename)return ERROR_INVALID_NAME;
    if(_wcsicmp(filename,expected_name))return ERROR_INVALID_NAME;
    for(unsigned attempt=0;attempt<8;++attempt){
        snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
        if(snapshot!=INVALID_HANDLE_VALUE || GetLastError()!=ERROR_BAD_LENGTH)break;
    }
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    if(!Module32FirstW(snapshot,&entry)){error=GetLastError();goto done;}
    do{
        if(_wcsicmp(entry.szModule,filename))continue;
        DWORD length=module_path(process,entry.hModule,full,ARRAYSIZE(full));
        if(!length){error=GetLastError();if(!error)error=ERROR_INVALID_HANDLE;goto done;}
        if(length>=ARRAYSIZE(full)){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
        if(_wcsicmp(full,canonical))continue;
        if(base){error=ERROR_DUP_NAME;goto done;}
        base=entry.modBaseAddr;
    }while(Module32NextW(snapshot,&entry));
    if(GetLastError()!=ERROR_NO_MORE_FILES){error=GetLastError();goto done;}
    if(!base){error=ERROR_MOD_NOT_FOUND;goto done;}
    error=win32_process_image_remote_header(process,base,&header);if(error)goto done;
    if(header.OptionalHeader.NumberOfRvaAndSizes<=IMAGE_DIRECTORY_ENTRY_EXPORT){error=ERROR_BAD_EXE_FORMAT;goto done;}
    IMAGE_DATA_DIRECTORY directory=header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    DWORD size=header.OptionalHeader.SizeOfImage;
    if(!directory.VirtualAddress || directory.Size<sizeof(exports) || !win32_process_image_remote_range(directory.VirtualAddress,directory.Size,size)){
        error=ERROR_BAD_EXE_FORMAT;goto done;
    }
    error=win32_process_image_remote_read(process,base,directory.VirtualAddress,&exports,sizeof(exports),size);if(error)goto done;
    if(!exports.NumberOfNames || exports.NumberOfNames>4096 || !exports.NumberOfFunctions || exports.NumberOfFunctions>4096 ||
       !win32_process_image_remote_range(exports.AddressOfNames,(SIZE_T)exports.NumberOfNames*sizeof(DWORD),size) ||
       !win32_process_image_remote_range(exports.AddressOfNameOrdinals,(SIZE_T)exports.NumberOfNames*sizeof(WORD),size) ||
       !win32_process_image_remote_range(exports.AddressOfFunctions,(SIZE_T)exports.NumberOfFunctions*sizeof(DWORD),size)){
        error=ERROR_BAD_EXE_FORMAT;goto done;
    }
    for(DWORD i=0;i<exports.NumberOfNames;++i){
        DWORD name_rva=0,function_rva=0;WORD ordinal=0;char name[256];
        error=win32_process_image_remote_read(process,base,exports.AddressOfNames+i*sizeof(DWORD),&name_rva,sizeof(name_rva),size);if(error)goto done;
        error=win32_process_image_remote_read(process,base,name_rva,name,target_bytes,size);if(error)goto done;
        if(memcmp(name,target,target_bytes))continue;
        if(found){error=ERROR_DUP_NAME;goto done;}
        error=win32_process_image_remote_read(process,base,exports.AddressOfNameOrdinals+i*sizeof(WORD),&ordinal,sizeof(ordinal),size);if(error)goto done;
        if(ordinal>=exports.NumberOfFunctions){error=ERROR_BAD_EXE_FORMAT;goto done;}
        error=win32_process_image_remote_read(process,base,exports.AddressOfFunctions+ordinal*sizeof(DWORD),&function_rva,sizeof(function_rva),size);if(error)goto done;
        if(!function_rva || !win32_process_image_remote_range(function_rva,1,size) ||
           (function_rva>=directory.VirtualAddress && function_rva-directory.VirtualAddress<directory.Size) ||
           (uintptr_t)base>UINTPTR_MAX-function_rva){error=ERROR_BAD_EXE_FORMAT;goto done;}
        found=base+function_rva;
        if(!win32_process_image_remote_executable(process,found,base)){error=ERROR_INVALID_ADDRESS;goto done;}
    }
    if(!found){error=ERROR_PROC_NOT_FOUND;goto done;}
    *address=found;
done:
    CloseHandle(snapshot);return error;
}
