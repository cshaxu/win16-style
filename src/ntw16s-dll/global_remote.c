#include "global_remote.h"
#include <tlhelp32.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

static DWORD image_header(HANDLE process,const void *base,IMAGE_NT_HEADERS *header)
{
    IMAGE_DOS_HEADER dos;SIZE_T count=0;
    if(!ReadProcessMemory(process,base,&dos,sizeof(dos),&count))return GetLastError();
    if(count!=sizeof(dos) || dos.e_magic!=IMAGE_DOS_SIGNATURE || dos.e_lfanew<64 || dos.e_lfanew>1024*1024)
        return ERROR_BAD_EXE_FORMAT;
    if((uintptr_t)base>UINTPTR_MAX-(uintptr_t)dos.e_lfanew)return ERROR_BAD_EXE_FORMAT;
    if(!ReadProcessMemory(process,(const BYTE *)base+dos.e_lfanew,header,sizeof(*header),&count))return GetLastError();
    if(count!=sizeof(*header) || header->Signature!=IMAGE_NT_SIGNATURE ||
       header->FileHeader.Machine!=(sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386) ||
       header->FileHeader.SizeOfOptionalHeader<sizeof(IMAGE_OPTIONAL_HEADER) ||
       header->OptionalHeader.Magic!=(sizeof(void*)==8?IMAGE_NT_OPTIONAL_HDR64_MAGIC:IMAGE_NT_OPTIONAL_HDR32_MAGIC) ||
       header->OptionalHeader.SizeOfImage<(DWORD)dos.e_lfanew+sizeof(*header))return ERROR_BAD_EXE_FORMAT;
    return ERROR_SUCCESS;
}

static BOOL executable_image(HANDLE process,const void *address,const void *base)
{
    MEMORY_BASIC_INFORMATION info;DWORD protection;
    if(!VirtualQueryEx(process,address,&info,sizeof(info)) || info.AllocationBase!=base ||
       info.State!=MEM_COMMIT || info.Type!=MEM_IMAGE || (info.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return FALSE;
    protection=info.Protect&0xff;
    return protection==PAGE_EXECUTE || protection==PAGE_EXECUTE_READ ||
           protection==PAGE_EXECUTE_READWRITE || protection==PAGE_EXECUTE_WRITECOPY;
}

DWORD ntw16s_remote_loadlibrary(HANDLE process,DWORD pid,void **address)
{
    HMODULE kernel=GetModuleHandleW(L"kernel32.dll"),local=NULL;FARPROC function;
    WCHAR path[MAX_PATH],system[MAX_PATH],wow[MAX_PATH];DWORD length,system_length,wow_length,error=ERROR_SUCCESS;
    HANDLE snapshot=INVALID_HANDLE_VALUE;MODULEENTRY32W entry={.dwSize=sizeof(entry)};
    IMAGE_NT_HEADERS local_header,remote_header;BYTE local_bytes[16],remote_bytes[16];SIZE_T count=0;
    uintptr_t offset;void *found=NULL;
    if(address)*address=NULL;
    if(!process || !pid || !address || GetProcessId(process)!=pid)return ERROR_INVALID_PARAMETER;
    function=kernel?GetProcAddress(kernel,"LoadLibraryW"):NULL;
    if(!function || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          (LPCWSTR)(void *)function,&local))return ERROR_PROC_NOT_FOUND;
    length=GetModuleFileNameW(local,path,ARRAYSIZE(path));system_length=GetSystemDirectoryW(system,ARRAYSIZE(system));
    wow_length=GetSystemWow64DirectoryW(wow,ARRAYSIZE(wow));
    if(!length || length>=ARRAYSIZE(path) || !system_length || system_length>=ARRAYSIZE(system))return ERROR_INSUFFICIENT_BUFFER;
    const WCHAR *base=wcsrchr(path,L'\\');
    if(!base || (_wcsicmp(base+1,L"kernelbase.dll") && _wcsicmp(base+1,L"kernel32.dll")) ||
       !((!_wcsnicmp(path,system,system_length) && path[system_length]==L'\\' && base==path+system_length) ||
         (wow_length && wow_length<ARRAYSIZE(wow) && !_wcsnicmp(path,wow,wow_length) && path[wow_length]==L'\\' && base==path+wow_length)))return ERROR_ACCESS_DENIED;
    error=image_header(GetCurrentProcess(),local,&local_header);if(error)return error;
    offset=(uintptr_t)(void *)function-(uintptr_t)local;
    if(offset>local_header.OptionalHeader.SizeOfImage ||
       sizeof(local_bytes)>local_header.OptionalHeader.SizeOfImage-offset)return ERROR_BAD_EXE_FORMAT;
    if(!executable_image(GetCurrentProcess(),(void *)function,local))return ERROR_INVALID_ADDRESS;
    if(!ReadProcessMemory(GetCurrentProcess(),(void *)function,local_bytes,sizeof(local_bytes),&count) || count!=sizeof(local_bytes))return ERROR_PARTIAL_COPY;
    for(unsigned attempt=0;attempt<8;++attempt){
        snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
        if(snapshot!=INVALID_HANDLE_VALUE || GetLastError()!=ERROR_BAD_LENGTH)break;
    }
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    if(!Module32FirstW(snapshot,&entry)){error=GetLastError();goto done;}
    do{
        if(_wcsicmp(entry.szExePath,path))continue;
        if(found){error=ERROR_DUP_NAME;goto done;}
        error=image_header(process,entry.modBaseAddr,&remote_header);if(error)goto done;
        if(remote_header.OptionalHeader.SizeOfImage!=local_header.OptionalHeader.SizeOfImage ||
           remote_header.FileHeader.TimeDateStamp!=local_header.FileHeader.TimeDateStamp ||
           entry.modBaseSize!=local_header.OptionalHeader.SizeOfImage){error=ERROR_REVISION_MISMATCH;goto done;}
        if((uintptr_t)entry.modBaseAddr>UINTPTR_MAX-offset){error=ERROR_INVALID_ADDRESS;goto done;}
        found=entry.modBaseAddr+offset;
        if(!executable_image(process,found,entry.modBaseAddr)){error=ERROR_INVALID_ADDRESS;goto done;}
        if(!ReadProcessMemory(process,found,remote_bytes,sizeof(remote_bytes),&count) || count!=sizeof(remote_bytes)){
            error=ERROR_PARTIAL_COPY;goto done;
        }
        if(memcmp(local_bytes,remote_bytes,sizeof(local_bytes))){error=ERROR_REVISION_MISMATCH;goto done;}
    }while(Module32NextW(snapshot,&entry));
    if(GetLastError()!=ERROR_NO_MORE_FILES){error=GetLastError();goto done;}
    if(!found){error=ERROR_MOD_NOT_FOUND;goto done;}
    *address=found;
done:
    CloseHandle(snapshot);return error;
}

static BOOL image_range(DWORD rva,SIZE_T bytes,DWORD image_size)
{return rva<=image_size && bytes<=image_size-rva;}

static DWORD read_at(HANDLE process,const BYTE *base,DWORD rva,void *value,SIZE_T bytes,DWORD image_size)
{
    SIZE_T count=0;
    if(!image_range(rva,bytes,image_size) || (uintptr_t)base>UINTPTR_MAX-rva)return ERROR_BAD_EXE_FORMAT;
    if(!ReadProcessMemory(process,base+rva,value,bytes,&count))return GetLastError();
    return count==bytes?ERROR_SUCCESS:ERROR_PARTIAL_COPY;
}

DWORD ntw16s_remote_global_thread(HANDLE process,DWORD pid,const WCHAR *path,void **address)
{
    static const char target[]="Ntw16sGlobalThread";
    typedef DWORD (WINAPI *module_path_fn)(HANDLE,HMODULE,LPWSTR,DWORD);
    module_path_fn module_path=(module_path_fn)(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"K32GetModuleFileNameExW");
    HANDLE snapshot=INVALID_HANDLE_VALUE;MODULEENTRY32W entry={.dwSize=sizeof(entry)};
    IMAGE_NT_HEADERS header;IMAGE_EXPORT_DIRECTORY exports;BYTE *base=NULL;DWORD error=ERROR_SUCCESS;
    WCHAR full[32752],canonical[32752],*filename=NULL;void *found=NULL;
    if(address)*address=NULL;
    if(!process || !pid || !path || !*path || !address || GetProcessId(process)!=pid)return ERROR_INVALID_PARAMETER;
    if(!module_path)return ERROR_PROC_NOT_FOUND;
    DWORD path_length=GetFullPathNameW(path,ARRAYSIZE(canonical),canonical,&filename);
    if(!path_length || path_length>=ARRAYSIZE(canonical) || !filename)return ERROR_INVALID_NAME;
    if(_wcsicmp(filename,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll"))return ERROR_INVALID_NAME;
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
    error=image_header(process,base,&header);if(error)goto done;
    if(header.OptionalHeader.NumberOfRvaAndSizes<=IMAGE_DIRECTORY_ENTRY_EXPORT){error=ERROR_BAD_EXE_FORMAT;goto done;}
    IMAGE_DATA_DIRECTORY directory=header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    DWORD size=header.OptionalHeader.SizeOfImage;
    if(!directory.VirtualAddress || directory.Size<sizeof(exports) || !image_range(directory.VirtualAddress,directory.Size,size)){
        error=ERROR_BAD_EXE_FORMAT;goto done;
    }
    error=read_at(process,base,directory.VirtualAddress,&exports,sizeof(exports),size);if(error)goto done;
    if(!exports.NumberOfNames || exports.NumberOfNames>4096 || !exports.NumberOfFunctions || exports.NumberOfFunctions>4096 ||
       !image_range(exports.AddressOfNames,(SIZE_T)exports.NumberOfNames*sizeof(DWORD),size) ||
       !image_range(exports.AddressOfNameOrdinals,(SIZE_T)exports.NumberOfNames*sizeof(WORD),size) ||
       !image_range(exports.AddressOfFunctions,(SIZE_T)exports.NumberOfFunctions*sizeof(DWORD),size)){
        error=ERROR_BAD_EXE_FORMAT;goto done;
    }
    for(DWORD i=0;i<exports.NumberOfNames;++i){
        DWORD name_rva=0,function_rva=0;WORD ordinal=0;char name[sizeof(target)];
        error=read_at(process,base,exports.AddressOfNames+i*sizeof(DWORD),&name_rva,sizeof(name_rva),size);if(error)goto done;
        error=read_at(process,base,name_rva,name,sizeof(name),size);if(error)goto done;
        if(memcmp(name,target,sizeof(target)))continue;
        if(found){error=ERROR_DUP_NAME;goto done;}
        error=read_at(process,base,exports.AddressOfNameOrdinals+i*sizeof(WORD),&ordinal,sizeof(ordinal),size);if(error)goto done;
        if(ordinal>=exports.NumberOfFunctions){error=ERROR_BAD_EXE_FORMAT;goto done;}
        error=read_at(process,base,exports.AddressOfFunctions+ordinal*sizeof(DWORD),&function_rva,sizeof(function_rva),size);if(error)goto done;
        if(!function_rva || !image_range(function_rva,1,size) ||
           (function_rva>=directory.VirtualAddress && function_rva-directory.VirtualAddress<directory.Size) ||
           (uintptr_t)base>UINTPTR_MAX-function_rva){error=ERROR_BAD_EXE_FORMAT;goto done;}
        found=base+function_rva;
        if(!executable_image(process,found,base)){error=ERROR_INVALID_ADDRESS;goto done;}
    }
    if(!found){error=ERROR_PROC_NOT_FOUND;goto done;}
    *address=found;
done:
    CloseHandle(snapshot);return error;
}
