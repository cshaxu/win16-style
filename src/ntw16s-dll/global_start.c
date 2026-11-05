#include "lib-win32/handle/duplicate.h"
#include "global_start.h"
#include "global_thread.h"
#include "global_session.h"
#include "global_remote.h"
#include <sddl.h>
#include <stdlib.h>
#include <wchar.h>

/* The target has already been checked to have the same user SID. Allow that
 * user to signal this one ready event even at a lower integrity level, without
 * changing a desktop, system setting, or the global session's permissions. */
static HANDLE ready_event(const WCHAR *name,SECURITY_ATTRIBUTES *security,DWORD *error)
{
    HANDLE token=NULL,event=NULL;TOKEN_USER *user=NULL;LPWSTR sid=NULL;PSECURITY_DESCRIPTOR descriptor=NULL;
    WCHAR *sddl=NULL;DWORD bytes=0;SECURITY_ATTRIBUTES local={sizeof(local),NULL,FALSE};
    if(security){SetLastError(ERROR_SUCCESS);event=CreateEventW(security,TRUE,FALSE,name);*error=GetLastError();return event;}
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)){*error=GetLastError();goto done;}
    if(GetTokenInformation(token,TokenUser,NULL,0,&bytes) || GetLastError()!=ERROR_INSUFFICIENT_BUFFER || !bytes){*error=ERROR_INVALID_DATA;goto done;}
    user=(TOKEN_USER *)malloc(bytes);if(!user){*error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    if(!GetTokenInformation(token,TokenUser,user,bytes,&bytes) || !ConvertSidToStringSidW(user->User.Sid,&sid)){*error=GetLastError();goto done;}
    size_t length=wcslen(sid)+64;sddl=(WCHAR *)calloc(length,sizeof(*sddl));
    if(!sddl){*error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    if(swprintf(sddl,length,L"D:P(A;;GA;;;SY)(A;;GA;;;%ls)S:(ML;;NW;;;LW)",sid)<0){*error=ERROR_INVALID_NAME;goto done;}
    if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl,SDDL_REVISION_1,&descriptor,NULL)){*error=GetLastError();goto done;}
    local.lpSecurityDescriptor=descriptor;SetLastError(ERROR_SUCCESS);event=CreateEventW(&local,TRUE,FALSE,name);*error=GetLastError();
done:
    if(descriptor)LocalFree(descriptor);
    if(sid)LocalFree(sid);
    if(token)CloseHandle(token);
    free(sddl);free(user);return event;
}

DWORD ntw16s_global_start_close(ntw16s_global_start *ticket)
{
    DWORD wait,error;
    if(!ticket)return ERROR_INVALID_PARAMETER;
    if(ticket->thread){
        wait=WaitForSingleObject(ticket->thread,0);
        if(wait==WAIT_TIMEOUT)return ERROR_BUSY;
        if(wait!=WAIT_OBJECT_0)return wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    }
    if(ticket->remote_packet){
        if(!ticket->process)return ERROR_INVALID_STATE;
        if(!VirtualFreeEx(ticket->process,ticket->remote_packet,0,MEM_RELEASE)){
            error=GetLastError();
            if(WaitForSingleObject(ticket->process,0)!=WAIT_OBJECT_0)return error;
        }
    }
    if(ticket->thread)CloseHandle(ticket->thread);
    if(ticket->ready)CloseHandle(ticket->ready);
    if(ticket->process)CloseHandle(ticket->process);
    ZeroMemory(ticket,sizeof(*ticket));return ERROR_SUCCESS;
}

DWORD ntw16s_global_start_begin(HANDLE process,DWORD pid,DWORD thread_id,DWORD cookie,
                                const WCHAR *dll_path,SECURITY_ATTRIBUTES *security,ntw16s_global_start *ticket)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],ready_name[NTW16S_GLOBAL_NAME_CAP+64];
    HANDLE gui=NULL;void *worker=NULL;DWORD error;SIZE_T written=0;
    ntw16s_global_thread_packet packet={sizeof(packet),1,thread_id,cookie};
    if(!process || !pid || !thread_id || !cookie || !dll_path || !ticket || GetProcessId(process)!=pid)return ERROR_INVALID_PARAMETER;
    if(ticket->process || ticket->thread || ticket->ready || ticket->remote_packet)return ERROR_BUSY;
    gui=OpenThread(THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,thread_id);
    if(!gui)return GetLastError();
    if(GetProcessIdOfThread(gui)!=pid || WaitForSingleObject(gui,0)!=WAIT_TIMEOUT){error=ERROR_INVALID_THREAD_ID;goto fail;}
    error=ntw16s_remote_global_thread(process,pid,dll_path,&worker);if(error)goto fail;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.thread.%lu.%lu.%lu",name,pid,thread_id,cookie)<0){
        error=ERROR_INVALID_NAME;goto fail;
    }
    ticket->ready=ready_event(ready_name,security,&error);
    if(!ticket->ready)goto fail;
    if(error==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto fail;}
    error=win32_handle_duplicate(GetCurrentProcess(),process,GetCurrentProcess(),&ticket->process);
    if(error)goto fail;
    ticket->remote_packet=VirtualAllocEx(ticket->process,NULL,sizeof(packet),MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!ticket->remote_packet){error=GetLastError();goto fail;}
    if(!WriteProcessMemory(ticket->process,ticket->remote_packet,&packet,sizeof(packet),&written) || written!=sizeof(packet)){
        error=GetLastError();if(!error)error=ERROR_PARTIAL_COPY;goto fail;
    }
    ticket->thread=CreateRemoteThread(ticket->process,NULL,0,(LPTHREAD_START_ROUTINE)worker,ticket->remote_packet,0,NULL);
    if(!ticket->thread){error=GetLastError();goto fail;}
    CloseHandle(gui);return ERROR_SUCCESS;
fail:
    if(gui)CloseHandle(gui);
    {DWORD release=ntw16s_global_start_close(ticket);return release?release:error;}
}

DWORD ntw16s_global_start_ready(ntw16s_global_start *ticket,DWORD timeout)
{
    if(!ticket || !ticket->process || !ticket->thread || !ticket->ready)return ERROR_INVALID_PARAMETER;
    HANDLE waits[]={ticket->ready,ticket->thread,ticket->process};
    DWORD wait=WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,timeout);
    if(wait==WAIT_TIMEOUT)return ERROR_TIMEOUT;
    if(wait==WAIT_OBJECT_0+2)return ERROR_PROCESS_ABORTED;
    if(wait==WAIT_OBJECT_0+1){DWORD code=0;if(!GetExitCodeThread(ticket->thread,&code))return GetLastError();return code?code:ERROR_INVALID_STATE;}
    if(wait!=WAIT_OBJECT_0)return wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    if(WaitForSingleObject(ticket->process,0)!=WAIT_TIMEOUT || WaitForSingleObject(ticket->thread,0)!=WAIT_TIMEOUT)return ERROR_INVALID_STATE;
    /* The worker copies its packet before setting ready and never reads it
     * afterwards, so it is safe to release while the hook remains active. */
    if(ticket->remote_packet){
        if(!VirtualFreeEx(ticket->process,ticket->remote_packet,0,MEM_RELEASE))return GetLastError();
        ticket->remote_packet=NULL;
    }
    return ERROR_SUCCESS;
}
