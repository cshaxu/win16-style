#include "ntw16s-dll/global/attach.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/global/remote.h"
#include "lib-win32/process-global/module.h"
#include "lib-win32/process-global/transaction.h"
#include "lib-win32/process-global/remote_loader.h"
#include "lib-win32/process-global/remote_export.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <tlhelp32.h>

#define ATTACH_LOADING WIN32_PROCESS_GLOBAL_LOADING
#define ATTACH_STARTING WIN32_PROCESS_GLOBAL_STARTING
#define ATTACH_ACTIVE WIN32_PROCESS_GLOBAL_ACTIVE

DWORD ntw16s_global_attach_close(ntw16s_global_attach *ticket)
{
    DWORD error;
    if(!ticket)return ERROR_INVALID_PARAMETER;
    error=win32_process_global_worker_close(&ticket->start);if(error)return error;
    error=win32_process_global_load_discard(&ticket->load);if(error)return error;
    if(ticket->process)CloseHandle(ticket->process);
    free(ticket->dll_path);ZeroMemory(ticket,sizeof(*ticket));return ERROR_SUCCESS;
}

static DWORD revalidate(ntw16s_global_attach *ticket)
{return ticket->window?win32_process_global_policy_revalidate(ticket->process,ticket->window,&ticket->identity):
                       win32_process_global_policy_revalidate_thread(ticket->process,&ticket->identity);}

static DWORD mapped_identity(HANDLE process,DWORD pid,HMODULE remote,const WCHAR *name,const WCHAR *path)
{return win32_process_global_mapped_identity(process,pid,remote,name,path,"Ntw16sGlobalThread",ntw16s_remote_global_thread);}

static DWORD begin(const win32_process_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket)
{
    DWORD error;size_t length;
    if(!policy || !pid || !thread_id || !cookie || !dll_path || !*dll_path || !ticket)return ERROR_INVALID_PARAMETER;
    if(ticket->process || ticket->dll_path || ticket->phase)return ERROR_BUSY;
    error=window?ntw16s_global_policy_inspect(policy,window,pid,thread_id,machine,&ticket->identity):
                 ntw16s_global_policy_inspect_thread(policy,pid,thread_id,machine,&ticket->identity);if(error)return error;
    ticket->process=OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|
                                PROCESS_VM_WRITE|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid);
    if(!ticket->process){error=GetLastError();goto fail;}
    ticket->window=window;error=revalidate(ticket);if(error)goto fail;
    length=wcslen(dll_path);if(length>=32752){error=ERROR_FILENAME_EXCED_RANGE;goto fail;}
    ticket->dll_path=(WCHAR *)calloc(32752,sizeof(WCHAR));if(!ticket->dll_path){error=ERROR_NOT_ENOUGH_MEMORY;goto fail;}
    error=win32_process_global_select_path(ticket->process,pid,dll_path,ticket->dll_path,32752,win32_process_global_files_equal,mapped_identity);if(error)goto fail;
    ticket->window=window;ticket->cookie=cookie;
    {WCHAR *slash=wcsrchr(ticket->dll_path,L'\\');WCHAR *shared;
     if(!slash){error=ERROR_INVALID_NAME;goto fail;}
     size_t prefix=(size_t)(slash-ticket->dll_path)+1;
     shared=(WCHAR *)calloc(prefix+32,sizeof(WCHAR));if(!shared){error=ERROR_NOT_ENOUGH_MEMORY;goto fail;}
     memcpy(shared,ticket->dll_path,prefix*sizeof(WCHAR));
     wcscpy(shared+prefix,sizeof(void*)==8?L"ntwshk64.dll":L"ntwshk32.dll");
     error=win32_process_global_load_begin(ticket->process,pid,shared,sizeof(void*)==8?L"ntwshk64.dll":L"ntwshk32.dll",win32_process_global_remote_loadlibrary,&ticket->load);
     free(shared);if(error)goto fail;ticket->loading_shared=TRUE;}
    ticket->phase=ATTACH_LOADING;return ERROR_SUCCESS;
fail:
    {DWORD release=ntw16s_global_attach_close(ticket);return release?release:error;}
}

DWORD ntw16s_global_attach_begin(const win32_process_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket)
{return window?begin(policy,window,pid,thread_id,machine,cookie,dll_path,ticket):ERROR_INVALID_PARAMETER;}
DWORD ntw16s_global_attach_begin_thread(const win32_process_global_policy *policy,DWORD pid,DWORD thread_id,
                                       WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket)
{return begin(policy,NULL,pid,thread_id,machine,cookie,dll_path,ticket);}

static DWORD control(ntw16s_global_attach *ticket,LPARAM action)
{
    DWORD error=win32_process_global_policy_revalidate(ticket->process,ticket->window,&ticket->identity);
    if(error)return error;
    UINT message=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);if(!message)return GetLastError();
    DWORD_PTR ignored=0;
    if(!SendMessageTimeoutW(ticket->window,message,ticket->cookie,action,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,500,&ignored)){
        error=GetLastError();return error?error:ERROR_TIMEOUT;
    }
    if(action==NTW16S_GLOBAL_APPLY){
        DWORD_PTR ack=0;
        if(!SendMessageTimeoutW(ticket->window,message,ticket->cookie,NTW16S_GLOBAL_QUERY,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,500,&ack)){
            error=GetLastError();return error?error:ERROR_TIMEOUT;
        }
        if(ack!=NTW16S_GLOBAL_ATTACHED)return ERROR_NOT_READY;
    }
    return ERROR_SUCCESS;
}

typedef struct attach_steps {ntw16s_global_attach *ticket;SECURITY_ATTRIBUTES *security;} attach_steps;
static DWORD shared_export(HANDLE process,DWORD pid,const WCHAR *path,void **address)
{return win32_process_global_remote_export(process,pid,path,sizeof(void*)==8?L"ntwshk64.dll":L"ntwshk32.dll","process_hook_startup_begin",address);}
static DWORD step_load(void *context)
{
    attach_steps *s=(attach_steps *)context;ntw16s_global_attach *t=s->ticket;void *worker=NULL;DWORD error;
    if(!t->loading_shared)return ntw16s_global_load_finish(&t->load,0,&worker);
    error=win32_process_global_load_finish(&t->load,0,shared_export,&worker);if(error)return error;
    error=revalidate(t);if(error)return error;
    error=ntw16s_global_load_begin(t->process,t->identity.pid,t->dll_path,&t->load);if(error)return error;
    t->loading_shared=FALSE;return ERROR_TIMEOUT;
}
static DWORD step_validate(void *context)
{return revalidate(((attach_steps *)context)->ticket);}
static DWORD step_start(void *context)
{
    attach_steps *s=(attach_steps *)context;ntw16s_global_attach *t=s->ticket;
    return ntw16s_global_start_begin(t->process,t->identity.pid,t->identity.thread_id,t->cookie,t->dll_path,s->security,&t->start);
}
static DWORD step_ready(void *context)
{return win32_process_global_worker_ready(&((attach_steps *)context)->ticket->start,0);}
static DWORD step_apply(void *context)
{return control(((attach_steps *)context)->ticket,NTW16S_GLOBAL_APPLY);}
DWORD ntw16s_global_attach_advance(ntw16s_global_attach *ticket,SECURITY_ATTRIBUTES *security)
{
    static const win32_process_global_transaction_ops ops={step_load,step_validate,step_start,step_ready,step_apply};
    if(!ticket || !ticket->process || !ticket->dll_path || !ticket->phase)return ERROR_INVALID_PARAMETER;
    attach_steps steps={ticket,security};
    return win32_process_global_transaction_advance(&ticket->phase,&ticket->control_error,ticket->window!=NULL,&steps,&ops);
}

DWORD ntw16s_global_attach_remove(ntw16s_global_attach *ticket)
{
    if(!ticket || ticket->phase!=ATTACH_ACTIVE)return ERROR_INVALID_STATE;
    return control(ticket,NTW16S_GLOBAL_REMOVE);
}

DWORD ntw16s_global_attach_apply(ntw16s_global_attach *ticket)
{
    if(!ticket || ticket->phase!=ATTACH_ACTIVE)return ERROR_INVALID_STATE;
    ticket->control_error=control(ticket,NTW16S_GLOBAL_APPLY);
    return ticket->control_error;
}
