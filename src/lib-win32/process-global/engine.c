#include "engine.h"
static BOOL next(void *context)
{return win32_process_global_targets_next(&((win32_process_global_engine *)context)->targets);}
DWORD win32_process_global_engine_start(win32_process_global_engine *e,HDESK desktop,HANDLE quit,HANDLE stop,HANDLE work,HANDLE changed,
    unsigned count,SIZE_T size,const win32_process_global_target_ops *ops,void *context,DWORD timeout)
{
    if(!e || e->owner || !desktop || !quit || !stop || !work || !changed)return ERROR_INVALID_PARAMETER;
    e->owner=GetCurrentThreadId();e->quit=quit;e->stop=stop;e->changed=changed;
    DWORD error=win32_process_global_targets_init(&e->targets,work,changed,size,ops,context);
    if(error)return error;
    e->initialized=TRUE;
    return win32_process_global_dispatch_pool_start(&e->pool,count,desktop,quit,stop,work,next,e,timeout);
}
void win32_process_global_engine_submit(win32_process_global_engine *e,DWORD pid,DWORD tid,HWND window)
{win32_process_global_targets_submit(&e->targets,pid,tid,window);}
BOOL win32_process_global_engine_contains(win32_process_global_engine *e,DWORD pid,DWORD tid)
{return win32_process_global_targets_contains(&e->targets,pid,tid);}
BOOL win32_process_global_engine_has_process(win32_process_global_engine *e,DWORD pid)
{return win32_process_global_targets_has_process(&e->targets,pid);}
void win32_process_global_engine_stats_read(win32_process_global_engine *e,win32_process_global_engine_stats *s)
{
    if(!s)return;
    ZeroMemory(s,sizeof(*s));if(!e || !e->initialized)return;
    s->queued=InterlockedCompareExchange(&e->targets.queued,0,0);
    s->active=InterlockedCompareExchange(&e->targets.active,0,0);
    s->failed=InterlockedCompareExchange(&e->targets.failed,0,0);
    s->overflow=InterlockedCompareExchange(&e->targets.overflow,0,0);
}
DWORD win32_process_global_engine_wait_idle(win32_process_global_engine *e,DWORD timeout)
{
    if(!e || !e->initialized || e->owner!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    ULONGLONG deadline=GetTickCount64()+timeout;HANDLE waits[]={e->stop,e->quit,e->changed};
    for(;;){
        if(win32_process_global_targets_idle(&e->targets))return ERROR_SUCCESS;
        ULONGLONG now=GetTickCount64();if(now>=deadline)return ERROR_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,(DWORD)(deadline-now),QS_ALLINPUT);
        if(result==WAIT_OBJECT_0 || result==WAIT_OBJECT_0+1)return ERROR_CANCELLED;
        if(result==WAIT_TIMEOUT)return ERROR_TIMEOUT;
        if(result==WAIT_FAILED)return GetLastError();
        if(result==WAIT_OBJECT_0+ARRAYSIZE(waits)){MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}}
        else if(result!=WAIT_OBJECT_0+2)return ERROR_INVALID_STATE;
    }
}
DWORD win32_process_global_engine_stop(win32_process_global_engine *e,DWORD timeout,win32_process_global_target_close_fn close,void *context)
{
    if(!e || (e->owner && e->owner!=GetCurrentThreadId()) || !close)return ERROR_INVALID_PARAMETER;
    if(e->quit)SetEvent(e->quit);
    ULONGLONG deadline=GetTickCount64()+timeout;
    DWORD error=win32_process_global_dispatch_pool_wait(&e->pool,deadline);if(error)return error;
    if(e->initialized){error=win32_process_global_targets_close(&e->targets,deadline,close,context);if(error)return error;}
    win32_process_global_dispatch_pool_release(&e->pool);ZeroMemory(e,sizeof(*e));return ERROR_SUCCESS;
}
