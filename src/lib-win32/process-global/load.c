#include "lib-win32/handle/duplicate.h"
#include "load.h"
#include "remote_memory.h"
#include "lib-win32/process-image/file_handle.h"

#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

static void close_ticket(win32_process_global_load *ticket)
{
    if(ticket->thread)CloseHandle(ticket->thread);
    if(ticket->file && ticket->file!=INVALID_HANDLE_VALUE)CloseHandle(ticket->file);
    if(ticket->process)CloseHandle(ticket->process);
    free(ticket->path);ZeroMemory(ticket,sizeof(*ticket));
}

DWORD win32_process_global_load_discard(win32_process_global_load *ticket)
{
    DWORD error;
    if(!ticket)return ERROR_INVALID_PARAMETER;
    error=win32_process_global_remote_release(ticket->thread,ticket->process,ticket->remote_path);
    if(error)return error;
    close_ticket(ticket);return ERROR_SUCCESS;
}
DWORD win32_process_global_load_abandon(win32_process_global_load *ticket)
{
    if(!ticket || !ticket->thread || !ticket->process || !ticket->remote_path)return ERROR_INVALID_PARAMETER;
    DWORD wait=WaitForSingleObject(ticket->thread,0);
    if(wait!=WAIT_TIMEOUT)return wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    close_ticket(ticket);return ERROR_SUCCESS;
}

DWORD win32_process_global_load_begin(HANDLE process,DWORD pid,const WCHAR *path,const WCHAR *expected_name,win32_process_global_loader_fn resolve_loader,win32_process_global_load *ticket)
{
    void *loader=NULL;DWORD error,length;SIZE_T bytes;WCHAR *filename=NULL;
    if(!process || !pid || !path || !*path || !expected_name || !resolve_loader || !ticket || GetProcessId(process)!=pid)return ERROR_INVALID_PARAMETER;
    if(ticket->process || ticket->thread || ticket->file || ticket->remote_path || ticket->path || ticket->pid)return ERROR_BUSY;
    ticket->path=(WCHAR *)calloc(32752,sizeof(WCHAR));if(!ticket->path)return ERROR_NOT_ENOUGH_MEMORY;
    length=GetFullPathNameW(path,32752,ticket->path,&filename);
    if(!length){error=GetLastError();if(!error)error=ERROR_INVALID_NAME;goto fail;}
    if(length>=32752 || !filename || _wcsicmp(filename,expected_name)){
        error=ERROR_INVALID_NAME;goto fail;
    }
    error=win32_handle_duplicate(GetCurrentProcess(),process,GetCurrentProcess(),&ticket->process);
    if(error)goto fail;
    ticket->pid=pid;
    ticket->file=CreateFileW(ticket->path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OPEN_REPARSE_POINT,NULL);
    if(ticket->file==INVALID_HANDLE_VALUE){error=GetLastError();goto fail;}
    error=win32_process_image_validate_dll_handle(ticket->file);if(error)goto fail;
    error=resolve_loader(ticket->process,pid,&loader);if(error)goto fail;
    bytes=((SIZE_T)length+1)*sizeof(WCHAR);
    ticket->remote_path=VirtualAllocEx(ticket->process,NULL,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!ticket->remote_path){error=GetLastError();goto fail;}
    error=win32_process_global_remote_write(ticket->process,ticket->remote_path,ticket->path,bytes);
    if(error)goto fail;
    ticket->thread=CreateRemoteThread(ticket->process,NULL,0,(LPTHREAD_START_ROUTINE)loader,ticket->remote_path,0,NULL);
    if(!ticket->thread){error=GetLastError();goto fail;}
    return ERROR_SUCCESS;
fail:
    {DWORD release_error=win32_process_global_load_discard(ticket);return release_error?release_error:error;}
}

DWORD win32_process_global_load_finish(win32_process_global_load *ticket,DWORD timeout,win32_process_global_resolve_fn resolve,void **worker)
{
    DWORD wait,error;void *resolved=NULL;
    if(worker)*worker=NULL;
    if(!ticket || !ticket->process || !ticket->thread || !ticket->remote_path || !ticket->path || !resolve || !worker)return ERROR_INVALID_PARAMETER;
    wait=WaitForSingleObject(ticket->thread,timeout);
    if(wait==WAIT_TIMEOUT)return ERROR_TIMEOUT;
    if(wait!=WAIT_OBJECT_0)return wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    if(WaitForSingleObject(ticket->process,0)==WAIT_OBJECT_0)error=ERROR_PROCESS_ABORTED;
    else{
        error=resolve(ticket->process,ticket->pid,ticket->path,&resolved);
        if(error && WaitForSingleObject(ticket->process,0)==WAIT_OBJECT_0)error=ERROR_PROCESS_ABORTED;
    }
    DWORD release_error=win32_process_global_load_discard(ticket);
    if(release_error)return release_error;
    if(!error)*worker=resolved;
    return error;
}
