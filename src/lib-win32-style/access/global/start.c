#include "start.h"
#include "lib-win32/process-global/remote_export.h"
#include "lib-win32/process-global/identity.h"
#include <wchar.h>

/* The target has already been checked to have the same user SID. Allow that
 * user to signal this one ready event even at a lower integrity level, without
 * changing a desktop, system setting, or the global session's permissions. */
static HANDLE ready_event(const WCHAR *name,SECURITY_ATTRIBUTES *security,DWORD *error)
{
    HANDLE event;PSECURITY_DESCRIPTOR descriptor=NULL;SECURITY_ATTRIBUTES local={sizeof(local),NULL,FALSE};
    if(!security){*error=win32_process_global_user_security(TRUE,&descriptor);if(*error)return NULL;local.lpSecurityDescriptor=descriptor;security=&local;}
    SetLastError(ERROR_SUCCESS);event=CreateEventW(security,TRUE,FALSE,name);*error=GetLastError();
    if(descriptor)LocalFree(descriptor);
    return event;
}

DWORD style_global_start_begin(const style_global_start_config *config,HANDLE process,DWORD pid,DWORD thread_id,DWORD cookie,
                                const WCHAR *dll_path,SECURITY_ATTRIBUTES *security,win32_process_global_worker *ticket)
{
    WCHAR name[STYLE_GLOBAL_NAME_CAP],ready_name[STYLE_GLOBAL_NAME_CAP+64];
    HANDLE gui=NULL;void *worker=NULL;DWORD error;
    style_global_thread_packet packet={sizeof(packet),1,thread_id,cookie};
    if(!process || !pid || !thread_id || !cookie || !dll_path || !ticket || GetProcessId(process)!=pid)return ERROR_INVALID_PARAMETER;
    if(ticket->process || ticket->thread || ticket->ready || ticket->remote_packet)return ERROR_BUSY;
    gui=OpenThread(THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,thread_id);
    if(!gui)return GetLastError();
    if(GetProcessIdOfThread(gui)!=pid || WaitForSingleObject(gui,0)!=WAIT_TIMEOUT){error=ERROR_INVALID_THREAD_ID;goto fail;}
    error=win32_process_global_remote_export(process,pid,dll_path,config->space->module,config->worker_export,&worker);if(error)goto fail;
    if(!style_global_session_name(config->space,name,ARRAYSIZE(name)) || swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.thread.%lu.%lu.%lu",name,pid,thread_id,cookie)<0){
        error=ERROR_INVALID_NAME;goto fail;
    }
    ticket->ready=ready_event(ready_name,security,&error);
    if(!ticket->ready)goto fail;
    if(error==ERROR_ALREADY_EXISTS)goto fail;
    error=win32_process_global_worker_begin(ticket,process,worker,&packet,sizeof(packet));
    if(error)goto fail;
    CloseHandle(gui);return ERROR_SUCCESS;
fail:
    if(gui)CloseHandle(gui);
    {DWORD release=win32_process_global_worker_close(ticket);return release?release:error;}
}
