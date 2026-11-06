#include "targets.h"
#include <stdlib.h>
static BOOL expired(void *owner,void *context)
{
    win32_process_global_targets *s=context;
    win32_process_global_target *t=owner;
    return t->state!=WIN32_TARGET_QUEUED && t->state!=WIN32_TARGET_WORKING &&
        s->ops.expired(t,s->context);
}
static void release(void *owner,void *context)
{
    win32_process_global_targets *s=context;win32_process_global_target *t=owner;
    if(t->state==WIN32_TARGET_ACTIVE)InterlockedDecrement(&s->active);
    --s->count;s->ops.dispose(t,s->context);free(t);
}
static void enqueue(win32_process_global_targets *s,win32_process_global_target *t)
{t->state=WIN32_TARGET_QUEUED;win32_process_global_queue_push(&s->queue,&t->queue_node);InterlockedIncrement(&s->queued);SetEvent(s->work);}
DWORD win32_process_global_targets_init(win32_process_global_targets *s,HANDLE work,HANDLE changed,SIZE_T size,const win32_process_global_target_ops *ops,void *context)
{
    if(!s || !work || !changed || size<sizeof(win32_process_global_target) || !ops || !ops->expired || !ops->active_finished || !ops->failed_retryable || !ops->execute || !ops->dispose)return ERROR_INVALID_PARAMETER;
    s->work=work;s->changed=changed;s->target_size=size;s->ops=*ops;s->context=context;
    win32_process_global_registry_init(&s->registry,s->buckets,WIN32_PROCESS_GLOBAL_TARGET_BUCKETS);return ERROR_SUCCESS;
}
void win32_process_global_targets_submit(win32_process_global_targets *s,DWORD pid,DWORD tid,HWND window)
{
    AcquireSRWLockExclusive(&s->lock);
    if(win32_process_global_registry_reap(&s->registry,s->count>=WIN32_PROCESS_GLOBAL_TARGET_LIMIT?WIN32_PROCESS_GLOBAL_TARGET_BUCKETS:8u,expired,release,s))SetEvent(s->changed);
    win32_process_global_target *t=win32_process_global_registry_find(&s->registry,pid,tid);
    if(t){
        if(t->state==WIN32_TARGET_ACTIVE && s->ops.active_finished(t,s->context)){t->window=window;t->attempts=0;InterlockedDecrement(&s->active);enqueue(s,t);}
        if(t->state==WIN32_TARGET_FAILED && t->attempts<2 && s->ops.failed_retryable(t,s->context)){t->window=window;enqueue(s,t);}
        ReleaseSRWLockExclusive(&s->lock);return;
    }
    if(s->count>=WIN32_PROCESS_GLOBAL_TARGET_LIMIT){InterlockedIncrement(&s->overflow);ReleaseSRWLockExclusive(&s->lock);SetEvent(s->changed);return;}
    t=calloc(1,s->target_size);if(!t){InterlockedIncrement(&s->failed);ReleaseSRWLockExclusive(&s->lock);SetEvent(s->changed);return;}
    t->pid=pid;t->thread_id=tid;t->window=window;
    win32_process_global_registry_insert(&s->registry,&t->registry_node,pid,tid,t);++s->count;enqueue(s,t);
    ReleaseSRWLockExclusive(&s->lock);
}
BOOL win32_process_global_targets_next(win32_process_global_targets *s)
{
    AcquireSRWLockExclusive(&s->lock);win32_process_global_target *t=(win32_process_global_target *)win32_process_global_queue_pop(&s->queue);
    if(t){t->state=WIN32_TARGET_WORKING;InterlockedDecrement(&s->queued);InterlockedIncrement(&s->working);}
    ReleaseSRWLockExclusive(&s->lock);if(!t)return FALSE;
    SRWLOCK *process_lock=&s->process_locks[t->pid%ARRAYSIZE(s->process_locks)];AcquireSRWLockExclusive(process_lock);
    BOOL retained=FALSE;DWORD error=s->ops.execute(t,s->context,&retained);ReleaseSRWLockExclusive(process_lock);
    AcquireSRWLockExclusive(&s->lock);++t->attempts;InterlockedDecrement(&s->working);
    if(!error || retained){t->state=WIN32_TARGET_ACTIVE;InterlockedIncrement(&s->active);if(error)InterlockedIncrement(&s->failed);}
    else{t->state=WIN32_TARGET_FAILED;InterlockedIncrement(&s->failed);}
    ReleaseSRWLockExclusive(&s->lock);SetEvent(s->changed);return TRUE;
}
BOOL win32_process_global_targets_contains(win32_process_global_targets *s,DWORD pid,DWORD tid)
{AcquireSRWLockShared(&s->lock);BOOL found=win32_process_global_registry_find(&s->registry,pid,tid)!=NULL;ReleaseSRWLockShared(&s->lock);return found;}
BOOL win32_process_global_targets_idle(win32_process_global_targets *s)
{AcquireSRWLockShared(&s->lock);BOOL idle=s->queued==0 && s->working==0;ReleaseSRWLockShared(&s->lock);return idle;}
BOOL win32_process_global_targets_has_process(win32_process_global_targets *s,DWORD pid)
{
    BOOL found=FALSE;AcquireSRWLockShared(&s->lock);
    for(unsigned i=0;i<WIN32_PROCESS_GLOBAL_TARGET_BUCKETS && !found;++i)
        for(win32_process_global_registry_node *n=s->buckets[i];n;n=n->next){
            win32_process_global_target *t=n->owner;
            if(t->pid==pid && t->state!=WIN32_TARGET_FAILED){found=TRUE;break;}
        }
    ReleaseSRWLockShared(&s->lock);return found;
}
DWORD win32_process_global_targets_close(win32_process_global_targets *s,ULONGLONG deadline,win32_process_global_target_close_fn close,void *context)
{
    if(!s || !close)return ERROR_INVALID_PARAMETER;
    for(unsigned i=0;i<WIN32_PROCESS_GLOBAL_TARGET_BUCKETS;++i)
        for(win32_process_global_registry_node *n=s->buckets[i];n;n=n->next){
            DWORD error=close(n->owner,context,deadline);if(error)return error;
        }
    win32_process_global_targets_dispose(s);return ERROR_SUCCESS;
}
void win32_process_global_targets_dispose(win32_process_global_targets *s)
{
    for(unsigned i=0;i<WIN32_PROCESS_GLOBAL_TARGET_BUCKETS;++i){win32_process_global_registry_node *n=s->buckets[i];
        while(n){win32_process_global_registry_node *next=n->next;win32_process_global_target *t=n->owner;s->ops.dispose(t,s->context);free(t);n=next;}s->buckets[i]=NULL;}
    ZeroMemory(s,sizeof(*s));
}
