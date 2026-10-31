#include "global_load.h"
#include "global_remote.h"
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

static DWORD validate_file(HANDLE file)
{
    IMAGE_DOS_HEADER dos;IMAGE_NT_HEADERS nt;LARGE_INTEGER pos,size;DWORD read=0;
    BY_HANDLE_FILE_INFORMATION info;
    if(!GetFileInformationByHandle(file,&info))return GetLastError();
    if(info.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT))return ERROR_ACCESS_DENIED;
    if(!GetFileSizeEx(file,&size))return GetLastError();
    if(!ReadFile(file,&dos,sizeof(dos),&read,NULL) || read!=sizeof(dos) ||
       dos.e_magic!=IMAGE_DOS_SIGNATURE || dos.e_lfanew<64 || dos.e_lfanew>1024*1024)return ERROR_BAD_EXE_FORMAT;
    pos.QuadPart=dos.e_lfanew;
    if(size.QuadPart<pos.QuadPart+(LONGLONG)sizeof(nt) || !SetFilePointerEx(file,pos,NULL,FILE_BEGIN) ||
       !ReadFile(file,&nt,sizeof(nt),&read,NULL) || read!=sizeof(nt))return ERROR_BAD_EXE_FORMAT;
    if(nt.Signature!=IMAGE_NT_SIGNATURE || !(nt.FileHeader.Characteristics&IMAGE_FILE_DLL) ||
       nt.FileHeader.Machine!=(sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386) ||
       nt.FileHeader.SizeOfOptionalHeader<sizeof(IMAGE_OPTIONAL_HEADER) ||
       nt.OptionalHeader.Magic!=(sizeof(void*)==8?IMAGE_NT_OPTIONAL_HDR64_MAGIC:IMAGE_NT_OPTIONAL_HDR32_MAGIC))return ERROR_BAD_EXE_FORMAT;
    return ERROR_SUCCESS;
}

static void close_ticket(ntw16s_global_load *ticket)
{
    if(ticket->thread)CloseHandle(ticket->thread);
    if(ticket->file && ticket->file!=INVALID_HANDLE_VALUE)CloseHandle(ticket->file);
    if(ticket->process)CloseHandle(ticket->process);
    free(ticket->path);ZeroMemory(ticket,sizeof(*ticket));
}

DWORD ntw16s_global_load_discard(ntw16s_global_load *ticket)
{
    DWORD wait,error;
    if(!ticket)return ERROR_INVALID_PARAMETER;
    if(ticket->thread){
        wait=WaitForSingleObject(ticket->thread,0);
        if(wait==WAIT_TIMEOUT)return ERROR_BUSY;
        if(wait!=WAIT_OBJECT_0)return wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    }
    if(ticket->remote_path){
        if(!ticket->process)return ERROR_INVALID_STATE;
        if(!VirtualFreeEx(ticket->process,ticket->remote_path,0,MEM_RELEASE)){
            error=GetLastError();
            if(WaitForSingleObject(ticket->process,0)!=WAIT_OBJECT_0)return error;
        }
    }
    close_ticket(ticket);return ERROR_SUCCESS;
}

DWORD ntw16s_global_load_begin(HANDLE process,DWORD pid,const WCHAR *path,ntw16s_global_load *ticket)
{
    void *loader=NULL;DWORD error,length;SIZE_T written=0,bytes;WCHAR *filename=NULL;
    if(!process || !pid || !path || !*path || !ticket || GetProcessId(process)!=pid)return ERROR_INVALID_PARAMETER;
    if(ticket->process || ticket->thread || ticket->file || ticket->remote_path || ticket->path || ticket->pid)return ERROR_BUSY;
    ticket->path=(WCHAR *)calloc(32752,sizeof(WCHAR));if(!ticket->path)return ERROR_NOT_ENOUGH_MEMORY;
    length=GetFullPathNameW(path,32752,ticket->path,&filename);
    if(!length || length>=32752 || !filename || _wcsicmp(filename,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll")){
        error=ERROR_INVALID_NAME;goto fail;
    }
    if(!DuplicateHandle(GetCurrentProcess(),process,GetCurrentProcess(),&ticket->process,0,FALSE,DUPLICATE_SAME_ACCESS)){
        error=GetLastError();goto fail;
    }
    ticket->pid=pid;
    ticket->file=CreateFileW(ticket->path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OPEN_REPARSE_POINT,NULL);
    if(ticket->file==INVALID_HANDLE_VALUE){error=GetLastError();goto fail;}
    error=validate_file(ticket->file);if(error)goto fail;
    error=ntw16s_remote_loadlibrary(ticket->process,pid,&loader);if(error)goto fail;
    bytes=((SIZE_T)length+1)*sizeof(WCHAR);
    ticket->remote_path=VirtualAllocEx(ticket->process,NULL,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!ticket->remote_path){error=GetLastError();goto fail;}
    if(!WriteProcessMemory(ticket->process,ticket->remote_path,ticket->path,bytes,&written) || written!=bytes){
        error=GetLastError();if(!error)error=ERROR_PARTIAL_COPY;goto fail;
    }
    ticket->thread=CreateRemoteThread(ticket->process,NULL,0,(LPTHREAD_START_ROUTINE)loader,ticket->remote_path,0,NULL);
    if(!ticket->thread){error=GetLastError();goto fail;}
    return ERROR_SUCCESS;
fail:
    {DWORD release_error=ntw16s_global_load_discard(ticket);return release_error?release_error:error;}
}

DWORD ntw16s_global_load_finish(ntw16s_global_load *ticket,DWORD timeout,void **worker)
{
    DWORD wait,error;void *resolved=NULL;
    if(worker)*worker=NULL;
    if(!ticket || !ticket->process || !ticket->thread || !ticket->remote_path || !ticket->path || !worker)return ERROR_INVALID_PARAMETER;
    wait=WaitForSingleObject(ticket->thread,timeout);
    if(wait==WAIT_TIMEOUT)return ERROR_TIMEOUT;
    if(wait!=WAIT_OBJECT_0)return wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    if(WaitForSingleObject(ticket->process,0)==WAIT_OBJECT_0)error=ERROR_PROCESS_ABORTED;
    else{
        error=ntw16s_remote_global_thread(ticket->process,ticket->pid,ticket->path,&resolved);
        if(error && WaitForSingleObject(ticket->process,0)==WAIT_OBJECT_0)error=ERROR_PROCESS_ABORTED;
    }
    DWORD release_error=ntw16s_global_load_discard(ticket);
    if(release_error)return release_error;
    if(!error)*worker=resolved;
    return error;
}
