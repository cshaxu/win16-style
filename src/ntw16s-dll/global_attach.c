#include "global_attach.h"
#include "global_session.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define ATTACH_LOADING 1u
#define ATTACH_STARTING 2u
#define ATTACH_ACTIVE 3u

DWORD ntw16s_global_attach_close(ntw16s_global_attach *ticket)
{
    DWORD error;
    if(!ticket)return ERROR_INVALID_PARAMETER;
    error=ntw16s_global_start_close(&ticket->start);if(error)return error;
    error=ntw16s_global_load_discard(&ticket->load);if(error)return error;
    if(ticket->process)CloseHandle(ticket->process);
    free(ticket->dll_path);ZeroMemory(ticket,sizeof(*ticket));return ERROR_SUCCESS;
}

DWORD ntw16s_global_attach_begin(const ntw16s_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket)
{
    DWORD error;size_t length;
    if(!policy || !window || !pid || !thread_id || !cookie || !dll_path || !*dll_path || !ticket)return ERROR_INVALID_PARAMETER;
    if(ticket->process || ticket->dll_path || ticket->phase)return ERROR_BUSY;
    error=ntw16s_global_policy_inspect(policy,window,pid,thread_id,machine,&ticket->identity);if(error)return error;
    ticket->process=OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|
                                PROCESS_VM_WRITE|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid);
    if(!ticket->process){error=GetLastError();goto fail;}
    error=ntw16s_global_policy_revalidate(ticket->process,window,&ticket->identity);if(error)goto fail;
    length=wcslen(dll_path);if(length>=32752){error=ERROR_FILENAME_EXCED_RANGE;goto fail;}
    ticket->dll_path=(WCHAR *)calloc(length+1,sizeof(WCHAR));if(!ticket->dll_path){error=ERROR_NOT_ENOUGH_MEMORY;goto fail;}
    memcpy(ticket->dll_path,dll_path,(length+1)*sizeof(WCHAR));ticket->window=window;ticket->cookie=cookie;
    error=ntw16s_global_load_begin(ticket->process,pid,ticket->dll_path,&ticket->load);if(error)goto fail;
    ticket->phase=ATTACH_LOADING;return ERROR_SUCCESS;
fail:
    {DWORD release=ntw16s_global_attach_close(ticket);return release?release:error;}
}

static DWORD control(ntw16s_global_attach *ticket,LPARAM action)
{
    DWORD error=ntw16s_global_policy_revalidate(ticket->process,ticket->window,&ticket->identity);
    if(error)return error;
    UINT message=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);if(!message)return GetLastError();
    DWORD_PTR ignored=0;
    if(!SendMessageTimeoutW(ticket->window,message,ticket->cookie,action,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,100,&ignored)){
        error=GetLastError();return error?error:ERROR_TIMEOUT;
    }
    return ERROR_SUCCESS;
}

DWORD ntw16s_global_attach_advance(ntw16s_global_attach *ticket,SECURITY_ATTRIBUTES *security)
{
    DWORD error;void *worker=NULL;
    if(!ticket || !ticket->process || !ticket->dll_path || !ticket->phase)return ERROR_INVALID_PARAMETER;
    if(ticket->phase==ATTACH_LOADING){
        error=ntw16s_global_load_finish(&ticket->load,0,&worker);
        if(error==ERROR_TIMEOUT)return ERROR_IO_PENDING;
        if(error)return error;
        error=ntw16s_global_policy_revalidate(ticket->process,ticket->window,&ticket->identity);if(error)return error;
        error=ntw16s_global_start_begin(ticket->process,ticket->identity.pid,ticket->identity.thread_id,ticket->cookie,
                                        ticket->dll_path,security,&ticket->start);
        if(error)return error;
        ticket->phase=ATTACH_STARTING;
    }
    if(ticket->phase==ATTACH_STARTING){
        error=ntw16s_global_start_ready(&ticket->start,0);
        if(error==ERROR_TIMEOUT)return ERROR_IO_PENDING;
        if(error)return error;
        ticket->phase=ATTACH_ACTIVE;
        ticket->control_error=control(ticket,NTW16S_GLOBAL_APPLY);
        return ticket->control_error;
    }
    return ticket->phase==ATTACH_ACTIVE?ticket->control_error:ERROR_INVALID_STATE;
}

DWORD ntw16s_global_attach_remove(ntw16s_global_attach *ticket)
{
    if(!ticket || ticket->phase!=ATTACH_ACTIVE)return ERROR_INVALID_STATE;
    return control(ticket,NTW16S_GLOBAL_REMOVE);
}
