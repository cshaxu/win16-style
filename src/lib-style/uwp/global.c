#include "global.h"
#include "target.h"
#include "lib-win32/process-global/dispatch.h"
#include "lib-win32/process-global/queue.h"
#include <objbase.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct frame_entry {
    win32_process_global_queue_node queue;
    struct frame_entry *next;
    style_uwp_target target;
    win32_process_global_identity host_identity;
    HANDLE application,host;
    DWORD result;
    BOOL queued,working,applied;
} frame_entry;
struct style_uwp_global {
    const win32_process_global_policy *policy;
    PCWSTR dll;
    DWORD owner;
    style_uwp_global_services services;
    SRWLOCK lock;
    frame_entry *frames;
    win32_process_global_queue queue;
    win32_process_global_dispatch_pool pool;
    HANDLE work,idle;
};
static BOOL alive(const frame_entry *e)
{
    DWORD pid=0;
    return WaitForSingleObject(e->application,0)==WAIT_TIMEOUT&&
        GetWindowThreadProcessId(e->target.core,&pid)&&pid==e->target.pid&&
        GetAncestor(e->target.core,GA_ROOT)==e->target.frame&&
        !win32_process_global_policy_revalidate(e->host,e->target.frame,&e->host_identity);
}
static void release_entry(frame_entry *e)
{if(e->application)CloseHandle(e->application);if(e->host)CloseHandle(e->host);free(e);}
static BOOL execute(void *context)
{
    style_uwp_global *g=context;
    AcquireSRWLockExclusive(&g->lock);
    frame_entry *e=(frame_entry *)win32_process_global_queue_pop(&g->queue);
    if(e){e->queued=FALSE;e->working=TRUE;}
    ReleaseSRWLockExclusive(&g->lock);
    if(!e)return FALSE;
    DWORD error=ERROR_CANCELLED;
    if(g->services.active(g->services.context)&&alive(e)){
        HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
        if(SUCCEEDED(hr)){error=g->services.change(&e->target,g->dll,TRUE,g->services.context);CoUninitialize();}
        else error=(DWORD)hr;
    }
    if(g->services.report)g->services.report(&e->target,error,g->services.context);
    AcquireSRWLockExclusive(&g->lock);e->result=error;e->applied=!error;e->working=FALSE;
    if(!g->queue.head)SetEvent(g->idle);
    ReleaseSRWLockExclusive(&g->lock);
    return TRUE;
}
DWORD style_uwp_global_start(const win32_process_global_policy *policy,PCWSTR dll,
    HDESK desktop,HANDLE quit,HANDLE stop,const style_uwp_global_services *services,style_uwp_global **result)
{
    if(!result||!policy||!dll||!desktop||!quit||!stop||!services||!services->active||!services->change||!services->host_image)return ERROR_INVALID_PARAMETER;
    *result=calloc(1,sizeof(**result));if(!*result)return ERROR_NOT_ENOUGH_MEMORY;
    style_uwp_global *g=*result;g->policy=policy;g->dll=dll;g->services=*services;g->owner=GetCurrentThreadId();
    g->work=CreateEventW(NULL,FALSE,FALSE,NULL);if(!g->work)return GetLastError();
    g->idle=CreateEventW(NULL,TRUE,TRUE,NULL);if(!g->idle)return GetLastError();
    return win32_process_global_dispatch_pool_start(&g->pool,1,desktop,quit,stop,g->work,execute,g,3000);
}
BOOL style_uwp_global_observe(style_uwp_global *g,HWND window)
{
    style_uwp_target target;
    if(!g||g->owner!=GetCurrentThreadId()||!style_uwp_target_window(window,&target))return FALSE;
    AcquireSRWLockExclusive(&g->lock);
    frame_entry **link=&g->frames,*found=NULL;
    while(*link){
        frame_entry *e=*link;
        if(!e->queued&&!e->working&&!alive(e)){*link=e->next;release_entry(e);continue;}
        if(e->target.frame==target.frame&&e->target.core==target.core&&e->target.pid==target.pid)found=e;
        link=&e->next;
    }
    if(!found){
        DWORD pid=0,thread=GetWindowThreadProcessId(target.frame,&pid);win32_process_global_identity identity;
        DWORD error=win32_process_global_policy_inspect(g->policy,target.frame,pid,thread,g->services.machine,&identity,g->services.host_image);
        if(!error){
            found=calloc(1,sizeof(*found));
            if(found){
                found->target=target;found->host_identity=identity;found->result=ERROR_IO_PENDING;
                found->application=OpenProcess(SYNCHRONIZE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,target.pid);
                found->host=OpenProcess(SYNCHRONIZE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
                if(!found->application||!found->host||!alive(found)){release_entry(found);found=NULL;}
                else {found->next=g->frames;g->frames=found;}
            }
        }
    }
    if(found&&!found->queued&&!found->working&&!found->applied){
        found->queued=TRUE;ResetEvent(g->idle);win32_process_global_queue_push(&g->queue,&found->queue);SetEvent(g->work);
    }
    ReleaseSRWLockExclusive(&g->lock);return TRUE;
}
DWORD style_uwp_global_wait_idle(style_uwp_global *g,DWORD timeout)
{
    if(!g)return ERROR_SUCCESS;
    if(g->owner!=GetCurrentThreadId())return ERROR_INVALID_THREAD_ID;
    ULONGLONG deadline=GetTickCount64()+timeout;
    for(;;){
        ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;
        DWORD result=MsgWaitForMultipleObjects(1,&g->idle,FALSE,left,QS_ALLINPUT);
        if(result==WAIT_OBJECT_0)return ERROR_SUCCESS;
        if(result==WAIT_TIMEOUT)return ERROR_TIMEOUT;
        if(result==WAIT_FAILED)return GetLastError();
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
}
void style_uwp_global_stats_read(style_uwp_global *g,LONG *queued,LONG *active,LONG *failed)
{
    if(!g)return;
    AcquireSRWLockShared(&g->lock);
    for(frame_entry *e=g->frames;e;e=e->next){
        if(e->queued||e->working)++*queued;
        else if(e->applied)++*active;
        else if(e->result!=ERROR_IO_PENDING)++*failed;
    }
    ReleaseSRWLockShared(&g->lock);
}
DWORD style_uwp_global_stop(style_uwp_global *g,DWORD timeout)
{
    if(!g||g->owner!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    ULONGLONG deadline=GetTickCount64()+timeout;
    DWORD error=win32_process_global_dispatch_pool_wait(&g->pool,deadline);if(error)return error;
    while(g->frames){
        frame_entry *e=g->frames;
        if(e->applied&&alive(e)){
            if(GetTickCount64()>=deadline)return ERROR_TIMEOUT;
            error=g->services.change(&e->target,g->dll,FALSE,g->services.context);
            if(error&&error!=ERROR_NOT_FOUND)return error;
        }
        g->frames=e->next;release_entry(e);
    }
    win32_process_global_dispatch_pool_release(&g->pool);
    if(g->work)CloseHandle(g->work);
    if(g->idle)CloseHandle(g->idle);
    free(g);return ERROR_SUCCESS;
}
