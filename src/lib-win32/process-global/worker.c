#include "worker.h"
#include "lib-win32/handle/duplicate.h"

DWORD win32_process_global_worker_begin(win32_process_global_worker *ticket,HANDLE process,void *worker,const void *packet,SIZE_T bytes)
{
    DWORD error;SIZE_T written=0;
    if(!ticket || !process || !worker || !packet || !bytes || !ticket->ready || ticket->process || ticket->thread || ticket->remote_packet)return ERROR_INVALID_PARAMETER;
    error=win32_handle_duplicate(GetCurrentProcess(),process,GetCurrentProcess(),&ticket->process);
    if(error)return error;
    ticket->remote_packet=VirtualAllocEx(ticket->process,NULL,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!ticket->remote_packet)return GetLastError();
    if(!WriteProcessMemory(ticket->process,ticket->remote_packet,packet,bytes,&written) || written!=bytes){
        error=GetLastError();return error?error:ERROR_PARTIAL_COPY;
    }
    ticket->thread=CreateRemoteThread(ticket->process,NULL,0,(LPTHREAD_START_ROUTINE)worker,ticket->remote_packet,0,NULL);
    return ticket->thread?ERROR_SUCCESS:GetLastError();
}

DWORD win32_process_global_worker_close(win32_process_global_worker *ticket)
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

DWORD win32_process_global_worker_ready(win32_process_global_worker *ticket,DWORD timeout)
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
