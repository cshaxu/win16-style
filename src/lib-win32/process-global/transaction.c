#include "transaction.h"
DWORD win32_process_global_transaction_advance(DWORD *phase,DWORD *control_error,BOOL apply,void *context,const win32_process_global_transaction_ops *ops)
{
    DWORD error;
    if(!phase || !control_error || !ops || !ops->load || !ops->validate || !ops->start || !ops->ready || !ops->apply)return ERROR_INVALID_PARAMETER;
    if(*phase==WIN32_PROCESS_GLOBAL_LOADING){
        error=ops->load(context);if(error==ERROR_TIMEOUT)return ERROR_IO_PENDING;if(error)return error;
        error=ops->validate(context);if(error)return error;
        error=ops->start(context);if(error)return error;
        *phase=WIN32_PROCESS_GLOBAL_STARTING;
    }
    if(*phase==WIN32_PROCESS_GLOBAL_STARTING){
        error=ops->ready(context);if(error==ERROR_TIMEOUT)return ERROR_IO_PENDING;if(error)return error;
        *phase=WIN32_PROCESS_GLOBAL_ACTIVE;
        if(!apply)return ERROR_SUCCESS;
        *control_error=ops->apply(context);return *control_error;
    }
    return *phase==WIN32_PROCESS_GLOBAL_ACTIVE?*control_error:ERROR_INVALID_STATE;
}

static DWORD cancellation(HANDLE quit,HANDLE stop,const win32_process_global_transaction_run_ops *ops,void *context)
{
    HANDLE handles[]={quit,stop};
    for(unsigned i=0;i<ARRAYSIZE(handles);++i){
        DWORD result=WaitForSingleObject(handles[i],0);
        if(result==WAIT_OBJECT_0)return ERROR_CANCELLED;
        if(result!=WAIT_TIMEOUT)return result==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    }
    return ops->active(context)?ERROR_SUCCESS:ERROR_CANCELLED;
}
DWORD win32_process_global_transaction_run(HANDLE quit,HANDLE stop,ULONGLONG deadline,ULONGLONG begin_deadline,
    BOOL retry_not_ready,const win32_process_global_transaction_run_ops *ops,void *context)
{
    if(!quit || !stop || !ops || !ops->active || !ops->begin || !ops->advance || !ops->pending || !ops->retry || !ops->apply)return ERROR_INVALID_PARAMETER;
    DWORD error;
    for(;;){
        if(!ops->active(context))return ERROR_CANCELLED;
        error=ops->begin(context);
        if(error!=ERROR_NOT_READY || !retry_not_ready || GetTickCount64()>=begin_deadline)break;
        HANDLE waits[]={quit,stop};DWORD wait=WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,20);
        if(wait==WAIT_OBJECT_0 || wait==WAIT_OBJECT_0+1)return ERROR_CANCELLED;
        if(wait!=WAIT_TIMEOUT)return wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    }
    if(error)return error;
    for(;;){
        error=cancellation(quit,stop,ops,context);if(error)return error;
        if(GetTickCount64()>=deadline)return ERROR_TIMEOUT;
        error=ops->advance(context);if(error!=ERROR_IO_PENDING)break;
        HANDLE waits[4]={quit,stop,NULL,NULL};unsigned count=ops->pending(context,&waits[2]);
        if(!count || count>2 || !waits[2] || (count==2 && !waits[3]))return ERROR_INVALID_STATE;
        ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;
        DWORD result=WaitForMultipleObjects(count+2,waits,FALSE,left);
        if(result==WAIT_TIMEOUT)return ERROR_TIMEOUT;
        if(result==WAIT_OBJECT_0 || result==WAIT_OBJECT_0+1)return ERROR_CANCELLED;
        if(result==WAIT_FAILED)return GetLastError();
        if(result>=WAIT_OBJECT_0+count+2)return ERROR_INVALID_STATE;
    }
    while(ops->retry(context,error) && GetTickCount64()<deadline){
        error=cancellation(quit,stop,ops,context);if(error)return error;
        error=ops->apply(context);
    }
    return error;
}
