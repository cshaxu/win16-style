#include "module.h"
#include <tlhelp32.h>
#include <string.h>
#include <wchar.h>

/* A file may have been replaced since the target mapped it. Check the mapped
 * image header as well as comparing the files before reusing an instance. */
DWORD win32_process_global_mapped_identity(HANDLE process,DWORD pid,HMODULE remote,const WCHAR *name,const WCHAR *path,const char *export_name,win32_process_global_resolve_fn resolve)
{
    HMODULE local=GetModuleHandleW(name);IMAGE_DOS_HEADER rd,ld;IMAGE_NT_HEADERS rn,ln;SIZE_T n=0;
    BYTE local_code[16],remote_code[16];void *remote_worker=NULL;FARPROC local_worker;
    if(!local || !remote)return ERROR_MOD_NOT_FOUND;
    if(!ReadProcessMemory(process,remote,&rd,sizeof(rd),&n) || n!=sizeof(rd) || rd.e_magic!=IMAGE_DOS_SIGNATURE || rd.e_lfanew<64 || rd.e_lfanew>1024*1024)return ERROR_BAD_EXE_FORMAT;
    memcpy(&ld,local,sizeof(ld));
    if(ld.e_magic!=IMAGE_DOS_SIGNATURE || ld.e_lfanew<64 || ld.e_lfanew>1024*1024)return ERROR_BAD_EXE_FORMAT;
    if(!ReadProcessMemory(process,(BYTE *)remote+rd.e_lfanew,&rn,sizeof(rn),&n) || n!=sizeof(rn))return ERROR_PARTIAL_COPY;
    memcpy(&ln,(BYTE *)local+ld.e_lfanew,sizeof(ln));
    if(rn.Signature!=IMAGE_NT_SIGNATURE || ln.Signature!=IMAGE_NT_SIGNATURE ||
       rn.FileHeader.Machine!=ln.FileHeader.Machine || rn.FileHeader.TimeDateStamp!=ln.FileHeader.TimeDateStamp ||
       rn.OptionalHeader.Magic!=ln.OptionalHeader.Magic || rn.OptionalHeader.SizeOfImage!=ln.OptionalHeader.SizeOfImage ||
       rn.OptionalHeader.CheckSum!=ln.OptionalHeader.CheckSum)return ERROR_REVISION_MISMATCH;
    local_worker=GetProcAddress(local,export_name);if(!local_worker)return ERROR_PROC_NOT_FOUND;
    DWORD error=resolve(process,pid,path,&remote_worker);if(error)return error;
    if(!ReadProcessMemory(GetCurrentProcess(),(const void *)local_worker,local_code,sizeof(local_code),&n) || n!=sizeof(local_code) ||
       !ReadProcessMemory(process,remote_worker,remote_code,sizeof(remote_code),&n) || n!=sizeof(remote_code))return ERROR_PARTIAL_COPY;
    if(memcmp(local_code,remote_code,sizeof(local_code)))return ERROR_REVISION_MISMATCH;
    return ERROR_SUCCESS;
}

/* Reuse the first same-named module already in the target. The target's
 * GetModuleHandle uses this same load order when the global hook fires. */
DWORD win32_process_global_select_path(HANDLE process,DWORD pid,const WCHAR *requested,WCHAR *selected,DWORD capacity,win32_process_global_files_equal_fn equal,win32_process_global_mapped_identity_fn identity)
{
    typedef DWORD (WINAPI *module_path_fn)(HANDLE,HMODULE,LPWSTR,DWORD);
    module_path_fn module_path=(module_path_fn)(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"K32GetModuleFileNameExW");
    const WCHAR *base=wcsrchr(requested,L'\\');HANDLE snapshot=INVALID_HANDLE_VALUE;MODULEENTRY32W e={.dwSize=sizeof(e)};
    DWORD error=ERROR_SUCCESS;BOOL found=FALSE;size_t length;HMODULE found_module=NULL;
    if(!base || !base[1] || !module_path)return ERROR_INVALID_PARAMETER;
    for(unsigned i=0;i<8;++i){snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);if(snapshot!=INVALID_HANDLE_VALUE || GetLastError()!=ERROR_BAD_LENGTH)break;}
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    if(!Module32FirstW(snapshot,&e)){error=GetLastError();goto done;}
    do{
        if(_wcsicmp(e.szModule,base+1))continue;
        DWORD n=module_path(process,e.hModule,selected,capacity);
        if(!n || n>=capacity){error=n?ERROR_INSUFFICIENT_BUFFER:GetLastError();goto done;}
        found=TRUE;found_module=e.hModule;break;
    }while(Module32NextW(snapshot,&e));
    if(!found){
        if(GetLastError()!=ERROR_NO_MORE_FILES){error=GetLastError();goto done;}
        length=wcslen(requested);if(length>=capacity){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
        memcpy(selected,requested,(length+1)*sizeof(WCHAR));
    }else if(_wcsicmp(selected,requested)){
        if(!equal(selected,requested))error=ERROR_REVISION_MISMATCH;
        else error=identity(process,pid,found_module,base+1,selected);
    }
done:
    CloseHandle(snapshot);return error;
}


BOOL win32_process_global_files_equal(const WCHAR *a,const WCHAR *b)
{
    HANDLE left=INVALID_HANDLE_VALUE,right=INVALID_HANDLE_VALUE;BOOL equal=FALSE;
    BY_HANDLE_FILE_INFORMATION li,ri;LARGE_INTEGER ls,rs;BYTE x[16384],y[16384];DWORD nx,ny;
    if(!a || !b)return FALSE;
    left=CreateFileW(a,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,NULL);
    if(left==INVALID_HANDLE_VALUE)goto done;
    right=CreateFileW(b,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,NULL);
    if(right==INVALID_HANDLE_VALUE)goto done;
    if(!GetFileInformationByHandle(left,&li) || !GetFileInformationByHandle(right,&ri) ||
       ((li.dwFileAttributes|ri.dwFileAttributes)&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) ||
       !GetFileSizeEx(left,&ls) || !GetFileSizeEx(right,&rs) || ls.QuadPart<=0 || ls.QuadPart!=rs.QuadPart)goto done;
    do{
        if(!ReadFile(left,x,sizeof(x),&nx,NULL) || !ReadFile(right,y,sizeof(y),&ny,NULL) || nx!=ny || memcmp(x,y,nx))goto done;
    }while(nx);
    equal=TRUE;
done:
    if(right!=INVALID_HANDLE_VALUE)CloseHandle(right);
    if(left!=INVALID_HANDLE_VALUE)CloseHandle(left);
    return equal;
}
