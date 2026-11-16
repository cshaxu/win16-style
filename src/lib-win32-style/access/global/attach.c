#include "attach.h"
#include "lib-win32/process-global/module.h"
#include "lib-win32/process-global/transaction.h"
#include "lib-win32/process-global/remote_loader.h"
#include "lib-win32/process-global/remote_export.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define ATTACH_LOADING WIN32_PROCESS_GLOBAL_LOADING
#define ATTACH_ACTIVE WIN32_PROCESS_GLOBAL_ACTIVE

DWORD style_global_attach_close(style_global_attach *ticket)
{
    DWORD error;
    if(!ticket)return ERROR_INVALID_PARAMETER;
    error=win32_process_global_worker_close(&ticket->start);if(error)return error;
    error=win32_process_global_load_discard(&ticket->load);if(error)return error;
    if(ticket->process)CloseHandle(ticket->process);
    free(ticket->dll_path);ZeroMemory(ticket,sizeof(*ticket));return ERROR_SUCCESS;
}

static DWORD revalidate(style_global_attach *ticket)
{return ticket->window?win32_process_global_policy_revalidate(ticket->process,ticket->window,&ticket->identity):
                       win32_process_global_policy_revalidate_thread(ticket->process,&ticket->identity);}

static DWORD resolve_worker(HANDLE process,DWORD pid,const WCHAR *path,void **address,void *context)
{
    const style_global_attach_config *config=context;
    return win32_process_global_remote_export(process,pid,path,config->start->space->module,config->start->worker_export,address);
}
static DWORD mapped_identity(HANDLE process,DWORD pid,HMODULE remote,const WCHAR *name,const WCHAR *path,void *context)
{
    const style_global_attach_config *config=context;
    return win32_process_global_mapped_identity_using(process,pid,remote,name,path,config->start->worker_export,resolve_worker,context);
}

DWORD style_global_attach_begin(const style_global_attach_config *config,const win32_process_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                WORD machine,DWORD cookie,const WCHAR *dll_path,style_global_attach *ticket)
{
    DWORD error;size_t length;
    if(!policy || !pid || !thread_id || !cookie || !dll_path || !*dll_path || !ticket)return ERROR_INVALID_PARAMETER;
    if(ticket->process || ticket->dll_path || ticket->phase)return ERROR_BUSY;
    error=window?win32_process_global_policy_inspect(policy,window,pid,thread_id,machine,&ticket->identity,config->allowed):
                 win32_process_global_policy_inspect_thread(policy,pid,thread_id,machine,&ticket->identity,config->allowed);if(error)return error;
    ticket->config=config;
    ticket->process=OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|
                                PROCESS_VM_WRITE|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid);
    if(!ticket->process){error=GetLastError();goto fail;}
    ticket->window=window;error=revalidate(ticket);if(error)goto fail;
    length=wcslen(dll_path);if(length>=32752){error=ERROR_FILENAME_EXCED_RANGE;goto fail;}
    ticket->dll_path=(WCHAR *)calloc(32752,sizeof(WCHAR));if(!ticket->dll_path){error=ERROR_NOT_ENOUGH_MEMORY;goto fail;}
    error=win32_process_global_select_path_using(ticket->process,pid,dll_path,ticket->dll_path,32752,win32_process_global_files_equal,mapped_identity,(void *)config);if(error)goto fail;
    ticket->window=window;ticket->cookie=cookie;
    {WCHAR *slash=wcsrchr(ticket->dll_path,L'\\');WCHAR *shared;
     if(!slash){error=ERROR_INVALID_NAME;goto fail;}
     size_t prefix=(size_t)(slash-ticket->dll_path)+1;
     shared=(WCHAR *)calloc(prefix+wcslen(config->shared_module)+1,sizeof(WCHAR));if(!shared){error=ERROR_NOT_ENOUGH_MEMORY;goto fail;}
     memcpy(shared,ticket->dll_path,prefix*sizeof(WCHAR));
     wcscpy(shared+prefix,config->shared_module);
     error=win32_process_global_load_begin(ticket->process,pid,shared,config->shared_module,win32_process_global_remote_loadlibrary,&ticket->load);
     free(shared);if(error)goto fail;ticket->loading_shared=TRUE;}
    ticket->phase=ATTACH_LOADING;return ERROR_SUCCESS;
fail:
    {DWORD release=style_global_attach_close(ticket);return release?release:error;}
}

static DWORD control(style_global_attach *ticket,LPARAM action)
{
    DWORD error=win32_process_global_policy_revalidate(ticket->process,ticket->window,&ticket->identity);
    if(error)return error;
    UINT message=RegisterWindowMessageW(ticket->config->control);if(!message)return GetLastError();
    DWORD_PTR ignored=0;
    if(!SendMessageTimeoutW(ticket->window,message,ticket->cookie,action,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,500,&ignored)){
        error=GetLastError();return error?error:ERROR_TIMEOUT;
    }
    if(action==ticket->config->apply){
        DWORD_PTR ack=0;
        if(!SendMessageTimeoutW(ticket->window,message,ticket->cookie,ticket->config->query,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,500,&ack)){
            error=GetLastError();return error?error:ERROR_TIMEOUT;
        }
        if(ack!=ticket->config->attached)return ERROR_NOT_READY;
    }
    return ERROR_SUCCESS;
}

typedef struct attach_steps {style_global_attach *ticket;SECURITY_ATTRIBUTES *security;} attach_steps;
static DWORD shared_export(HANDLE process,DWORD pid,const WCHAR *path,void **address,void *context)
{
    const style_global_attach_config *config=context;
    return win32_process_global_remote_export(process,pid,path,config->shared_module,config->shared_export,address);
}
static DWORD step_load(void *context)
{
    attach_steps *s=(attach_steps *)context;style_global_attach *t=s->ticket;void *worker=NULL;DWORD error;
    if(!t->loading_shared)return win32_process_global_load_finish_using(&t->load,0,resolve_worker,(void *)t->config,&worker);
    error=win32_process_global_load_finish_using(&t->load,0,shared_export,(void *)t->config,&worker);if(error)return error;
    error=revalidate(t);if(error)return error;
    error=win32_process_global_load_begin(t->process,t->identity.pid,t->dll_path,t->config->start->space->module,win32_process_global_remote_loadlibrary,&t->load);if(error)return error;
    t->loading_shared=FALSE;return ERROR_TIMEOUT;
}
static DWORD step_validate(void *context)
{return revalidate(((attach_steps *)context)->ticket);}
static DWORD step_start(void *context)
{
    attach_steps *s=(attach_steps *)context;style_global_attach *t=s->ticket;
    return style_global_start_begin(t->config->start,t->process,t->identity.pid,t->identity.thread_id,t->cookie,t->dll_path,s->security,&t->start);
}
static DWORD step_ready(void *context)
{return win32_process_global_worker_ready(&((attach_steps *)context)->ticket->start,0);}
static DWORD step_apply(void *context)
{style_global_attach *ticket=((attach_steps *)context)->ticket;return control(ticket,ticket->config->apply);}
DWORD style_global_attach_advance(style_global_attach *ticket,SECURITY_ATTRIBUTES *security)
{
    static const win32_process_global_transaction_ops ops={step_load,step_validate,step_start,step_ready,step_apply};
    if(!ticket || !ticket->process || !ticket->dll_path || !ticket->phase)return ERROR_INVALID_PARAMETER;
    attach_steps steps={ticket,security};
    return win32_process_global_transaction_advance(&ticket->phase,&ticket->control_error,ticket->window!=NULL,&steps,&ops);
}

DWORD style_global_attach_remove(style_global_attach *ticket)
{
    if(!ticket || ticket->phase!=ATTACH_ACTIVE)return ERROR_INVALID_STATE;
    return control(ticket,ticket->config->remove);
}

DWORD style_global_attach_apply(style_global_attach *ticket)
{
    if(!ticket || ticket->phase!=ATTACH_ACTIVE)return ERROR_INVALID_STATE;
    ticket->control_error=control(ticket,ticket->config->apply);
    return ticket->control_error;
}
