#include "dispatch.h"
DWORD WINAPI win32_process_global_dispatch_run(void *parameter)
{
    win32_process_global_dispatch *dispatch=(win32_process_global_dispatch *)parameter;
    if(!SetThreadDesktop(dispatch->desktop)){dispatch->error=GetLastError();SetEvent(dispatch->ready);return 1;}
    SetEvent(dispatch->ready);HANDLE waits[]={dispatch->quit,dispatch->stop,dispatch->work};
    for(;;){
        DWORD result=WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,INFINITE);
        if(result==WAIT_OBJECT_0 || result==WAIT_OBJECT_0+1)break;
        if(result!=WAIT_OBJECT_0+2)break;
        while(dispatch->next(dispatch->context)){
            if(WaitForSingleObject(dispatch->quit,0)==WAIT_OBJECT_0 || WaitForSingleObject(dispatch->stop,0)==WAIT_OBJECT_0)break;
        }
    }
    return 0;
}

DWORD win32_process_global_dispatch_pool_start(win32_process_global_dispatch_pool *pool,
    unsigned count, HDESK desktop, HANDLE quit, HANDLE stop, HANDLE work,
    win32_process_global_dispatch_next next, void *context, DWORD timeout)
{
    if(!pool || !count || count>MAXIMUM_WAIT_OBJECTS || !desktop || !quit || !stop || !work || !next || pool->count)
        return ERROR_INVALID_PARAMETER;
    pool->count=count;
    for(unsigned i=0;i<count;++i){
        pool->ready[i]=CreateEventW(NULL,TRUE,FALSE,NULL);
        if(!pool->ready[i])return GetLastError();
        pool->contexts[i]=(win32_process_global_dispatch){desktop,quit,stop,work,pool->ready[i],0,next,context};
        pool->threads[i]=CreateThread(NULL,0,win32_process_global_dispatch_run,&pool->contexts[i],0,NULL);
        if(!pool->threads[i])return GetLastError();
    }
    if(WaitForMultipleObjects(count,pool->ready,TRUE,timeout)!=WAIT_OBJECT_0)return ERROR_TIMEOUT;
    for(unsigned i=0;i<count;++i)if(pool->contexts[i].error)return pool->contexts[i].error;
    return ERROR_SUCCESS;
}
DWORD win32_process_global_dispatch_pool_wait(win32_process_global_dispatch_pool *pool, ULONGLONG deadline)
{
    if(!pool)return ERROR_INVALID_PARAMETER;
    for(unsigned i=0;i<pool->count;++i)if(pool->threads[i]){
        ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;
        DWORD wait=WaitForSingleObject(pool->threads[i],left);
        if(wait!=WAIT_OBJECT_0)return wait==WAIT_TIMEOUT?ERROR_TIMEOUT:(wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE);
    }
    return ERROR_SUCCESS;
}
void win32_process_global_dispatch_pool_release(win32_process_global_dispatch_pool *pool)
{
    if(!pool)return;
    for(unsigned i=0;i<pool->count;++i){if(pool->threads[i])CloseHandle(pool->threads[i]);if(pool->ready[i])CloseHandle(pool->ready[i]);}
    ZeroMemory(pool,sizeof(*pool));
}
