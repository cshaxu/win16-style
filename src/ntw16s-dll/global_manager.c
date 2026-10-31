#include "global_manager.h"
#include "global_attach.h"
#include "global_discover.h"
#include "global_session.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define MANAGER_WORKERS 4u
#define MANAGER_BUCKETS 257u
#define MANAGER_LIMIT 16384u
typedef enum entry_state {ENTRY_QUEUED,ENTRY_WORKING,ENTRY_ACTIVE,ENTRY_FAILED} entry_state;
typedef struct manager_entry {
    DWORD pid,thread_id;HWND window;entry_state state;unsigned attempts;
    ntw16s_global_attach attach;
    struct manager_entry *hash_next,*queue_next;
} manager_entry;
typedef struct worker_context {struct ntw16s_global_manager *manager;unsigned index;} worker_context;
struct ntw16s_global_manager {
    ntw16s_global_discover discover;ntw16s_global_policy policy;ntw16s_global_session *session;
    HANDLE mapping,stop,quit,work,changed,workers[MANAGER_WORKERS],ready[MANAGER_WORKERS];
    worker_context contexts[MANAGER_WORKERS];DWORD worker_errors[MANAGER_WORKERS];
    HDESK desktop;WCHAR *dll_path;WORD machine;DWORD owner_thread;SRWLOCK lock;
    manager_entry *buckets[MANAGER_BUCKETS],*queue_head,*queue_tail;unsigned count;
    volatile LONG queued,active,failed,overflow;
};

static void notify_change(ntw16s_global_manager *m){SetEvent(m->changed);}
static unsigned bucket(DWORD pid,DWORD tid){return (unsigned)((pid^(tid*33u))%MANAGER_BUCKETS);}
static void enqueue(ntw16s_global_manager *m,manager_entry *entry)
{
    entry->state=ENTRY_QUEUED;entry->queue_next=NULL;
    if(m->queue_tail)m->queue_tail->queue_next=entry;else m->queue_head=entry;
    m->queue_tail=entry;InterlockedIncrement(&m->queued);SetEvent(m->work);
}
static void discovered(HWND window,DWORD pid,DWORD thread_id,void *context)
{
    ntw16s_global_manager *m=(ntw16s_global_manager *)context;
    if(m->session->phase!=NTW16S_GLOBAL_ACTIVE || pid==m->session->owner_pid || pid==m->session->host_pid)return;
    AcquireSRWLockExclusive(&m->lock);
    unsigned index=bucket(pid,thread_id);manager_entry *entry=m->buckets[index];
    while(entry && (entry->pid!=pid || entry->thread_id!=thread_id))entry=entry->hash_next;
    if(entry){
        /* A finished worker may be a dead thread or a recycled thread ID. */
        if(entry->state==ENTRY_ACTIVE && entry->attach.start.thread &&
           WaitForSingleObject(entry->attach.start.thread,0)==WAIT_OBJECT_0){
            entry->window=window;entry->attempts=0;InterlockedDecrement(&m->active);enqueue(m,entry);
        }
        if(entry->state==ENTRY_FAILED && entry->attempts<2 && !entry->attach.process){entry->window=window;enqueue(m,entry);}
        ReleaseSRWLockExclusive(&m->lock);return;
    }
    if(m->count>=MANAGER_LIMIT){InterlockedIncrement(&m->overflow);ReleaseSRWLockExclusive(&m->lock);notify_change(m);return;}
    entry=(manager_entry *)calloc(1,sizeof(*entry));
    if(!entry){InterlockedIncrement(&m->failed);ReleaseSRWLockExclusive(&m->lock);notify_change(m);return;}
    entry->pid=pid;entry->thread_id=thread_id;entry->window=window;
    entry->hash_next=m->buckets[index];m->buckets[index]=entry;++m->count;enqueue(m,entry);
    ReleaseSRWLockExclusive(&m->lock);
}
typedef struct scan_state {ntw16s_global_manager *manager;manager_entry *entry;ULONGLONG deadline;} scan_state;
static BOOL CALLBACK notify_window(HWND window,LPARAM parameter)
{
    scan_state *scan=(scan_state *)parameter;DWORD pid=0;
    if(GetTickCount64()>=scan->deadline)return FALSE;
    if(GetWindowThreadProcessId(window,&pid)==scan->entry->thread_id && pid==scan->entry->pid && IsWindowVisible(window) &&
       !ntw16s_global_policy_revalidate(scan->entry->attach.process,window,&scan->entry->attach.identity)){
        UINT message=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);DWORD_PTR ignored=0;
        if(message)SendMessageTimeoutW(window,message,scan->manager->session->cookie,NTW16S_GLOBAL_APPLY,
                                      SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,30,&ignored);
    }
    return TRUE;
}
static BOOL CALLBACK notify_top(HWND window,LPARAM parameter)
{
    if(!notify_window(window,parameter))return FALSE;
    EnumChildWindows(window,notify_window,parameter);return GetTickCount64()<((scan_state *)parameter)->deadline;
}
static DWORD process_entry(ntw16s_global_manager *m,manager_entry *entry)
{
    ULONGLONG deadline=GetTickCount64()+5000;
    if(WaitForSingleObject(m->quit,0)==WAIT_OBJECT_0 || WaitForSingleObject(m->stop,0)==WAIT_OBJECT_0 ||
       m->session->phase!=NTW16S_GLOBAL_ACTIVE)return ERROR_CANCELLED;
    if(entry->attach.process){DWORD closed=ntw16s_global_attach_close(&entry->attach);if(closed)return closed;}
    DWORD error=ntw16s_global_attach_begin(&m->policy,entry->window,entry->pid,entry->thread_id,m->machine,
                                            m->session->cookie,m->dll_path,&entry->attach);
    if(error)return error;
    for(;;){
        if(WaitForSingleObject(m->quit,0)==WAIT_OBJECT_0 || WaitForSingleObject(m->stop,0)==WAIT_OBJECT_0 ||
           m->session->phase!=NTW16S_GLOBAL_ACTIVE)return ERROR_CANCELLED;
        error=ntw16s_global_attach_advance(&entry->attach,NULL);
        if(error!=ERROR_IO_PENDING)break;
        HANDLE waits[4]={m->quit,m->stop,NULL,NULL};DWORD count=2;
        if(entry->attach.load.thread)waits[count++]=entry->attach.load.thread;
        else if(entry->attach.start.ready && entry->attach.start.thread){waits[count++]=entry->attach.start.ready;waits[count++]=entry->attach.start.thread;}
        else return ERROR_INVALID_STATE;
        ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;
        DWORD result=WaitForMultipleObjects(count,waits,FALSE,left);
        if(result==WAIT_TIMEOUT)return ERROR_TIMEOUT;
        if(result==WAIT_OBJECT_0 || result==WAIT_OBJECT_0+1)return ERROR_CANCELLED;
        if(result==WAIT_FAILED)return GetLastError();
        if(result>=WAIT_OBJECT_0+count)return ERROR_INVALID_STATE;
    }
    /* The target worker is already installed when a control send times out.
     * Retry that message without reloading or installing another worker. */
    for(unsigned retry=0;error==ERROR_TIMEOUT && entry->attach.control_error==ERROR_TIMEOUT && retry<3;++retry){
        if(WaitForSingleObject(m->quit,0)==WAIT_OBJECT_0 || WaitForSingleObject(m->stop,0)==WAIT_OBJECT_0 ||
           m->session->phase!=NTW16S_GLOBAL_ACTIVE)return ERROR_CANCELLED;
        error=ntw16s_global_attach_apply(&entry->attach);
    }
    if(error)return error;
    scan_state scan={m,entry,GetTickCount64()+1000};EnumThreadWindows(entry->thread_id,notify_top,(LPARAM)&scan);
    return ERROR_SUCCESS;
}
static DWORD WINAPI worker(void *parameter)
{
    worker_context *context=(worker_context *)parameter;ntw16s_global_manager *m=context->manager;
    if(!SetThreadDesktop(m->desktop)){m->worker_errors[context->index]=GetLastError();SetEvent(m->ready[context->index]);return 1;}
    SetEvent(m->ready[context->index]);HANDLE waits[]={m->quit,m->stop,m->work};
    for(;;){
        DWORD result=WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,INFINITE);
        if(result==WAIT_OBJECT_0 || result==WAIT_OBJECT_0+1)break;
        if(result!=WAIT_OBJECT_0+2)break;
        for(;;){
            AcquireSRWLockExclusive(&m->lock);manager_entry *entry=m->queue_head;
            if(entry){m->queue_head=entry->queue_next;if(!m->queue_head)m->queue_tail=NULL;entry->state=ENTRY_WORKING;InterlockedDecrement(&m->queued);}
            ReleaseSRWLockExclusive(&m->lock);if(!entry)break;
            DWORD error=process_entry(m,entry);
            if(error)(void)ntw16s_global_attach_close(&entry->attach);
            AcquireSRWLockExclusive(&m->lock);++entry->attempts;
            if(!error){entry->state=ENTRY_ACTIVE;InterlockedIncrement(&m->active);}
            else{
                entry->state=ENTRY_FAILED;InterlockedIncrement(&m->failed);
            }
            ReleaseSRWLockExclusive(&m->lock);notify_change(m);
            if(WaitForSingleObject(m->quit,0)==WAIT_OBJECT_0 || WaitForSingleObject(m->stop,0)==WAIT_OBJECT_0)break;
        }
    }
    return 0;
}
HANDLE ntw16s_global_manager_changed(ntw16s_global_manager *m){return m?m->changed:NULL;}
void ntw16s_global_manager_stats_read(ntw16s_global_manager *m,ntw16s_global_manager_stats *s)
{
    if(!s)return;
    ZeroMemory(s,sizeof(*s));
    if(!m)return;
    s->queued=InterlockedCompareExchange(&m->queued,0,0);s->active=InterlockedCompareExchange(&m->active,0,0);
    s->failed=InterlockedCompareExchange(&m->failed,0,0);s->overflow=InterlockedCompareExchange(&m->overflow,0,0);
}
DWORD ntw16s_global_manager_stop(ntw16s_global_manager *m,DWORD timeout)
{
    if(!m || m->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    if(m->discover.hook){DWORD error=ntw16s_global_discover_stop(&m->discover);if(error)return error;}
    SetEvent(m->quit);ULONGLONG deadline=GetTickCount64()+timeout;
    for(unsigned i=0;i<MANAGER_WORKERS;++i)if(m->workers[i]){
        ULONGLONG now=GetTickCount64();DWORD remaining=now<deadline?(DWORD)(deadline-now):0;
        DWORD wait=WaitForSingleObject(m->workers[i],remaining);
        if(wait!=WAIT_OBJECT_0)return wait==WAIT_TIMEOUT?ERROR_TIMEOUT:(wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE);
    }
    for(unsigned i=0;i<MANAGER_BUCKETS;++i)for(manager_entry *e=m->buckets[i];e;e=e->hash_next){
        if(e->attach.start.thread){ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;WaitForSingleObject(e->attach.start.thread,left);}
        if(e->attach.load.thread){ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;WaitForSingleObject(e->attach.load.thread,left);}
        DWORD error=ntw16s_global_attach_close(&e->attach);if(error)return error;
    }
    for(unsigned i=0;i<MANAGER_BUCKETS;++i){manager_entry *e=m->buckets[i];while(e){manager_entry *next=e->hash_next;free(e);e=next;}}
    for(unsigned i=0;i<MANAGER_WORKERS;++i){if(m->workers[i])CloseHandle(m->workers[i]);if(m->ready[i])CloseHandle(m->ready[i]);}
    if(m->session)UnmapViewOfFile(m->session);
    if(m->mapping)CloseHandle(m->mapping);
    if(m->stop)CloseHandle(m->stop);
    if(m->quit)CloseHandle(m->quit);
    if(m->work)CloseHandle(m->work);
    if(m->changed)CloseHandle(m->changed);
    free(m->dll_path);free(m);return ERROR_SUCCESS;
}
DWORD ntw16s_global_manager_start(const WCHAR *dll_path,WORD machine,ntw16s_global_manager **result)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16];DWORD error=ERROR_SUCCESS;
    if(result)*result=NULL;
    if(!dll_path || !*dll_path || !result || machine!=(sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386))return ERROR_INVALID_PARAMETER;
    ntw16s_global_manager *m=(ntw16s_global_manager *)calloc(1,sizeof(*m));if(!m)return ERROR_NOT_ENOUGH_MEMORY;
    m->owner_thread=GetCurrentThreadId();m->desktop=GetThreadDesktop(m->owner_thread);m->machine=machine;
    size_t length=wcslen(dll_path);if(length>=32752){error=ERROR_FILENAME_EXCED_RANGE;goto fail;}
    m->dll_path=(WCHAR *)calloc(length+1,sizeof(WCHAR));if(!m->dll_path){error=ERROR_NOT_ENOUGH_MEMORY;goto fail;}
    memcpy(m->dll_path,dll_path,(length+1)*sizeof(WCHAR));
    if(!m->desktop || !ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0){error=ERROR_INVALID_NAME;goto fail;}
    error=ntw16s_global_policy_init(&m->policy);if(error)goto fail;
    m->mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(!m->mapping){error=GetLastError();goto fail;}
    m->session=(ntw16s_global_session *)MapViewOfFile(m->mapping,FILE_MAP_READ,0,0,sizeof(*m->session));
    if(!m->session){error=GetLastError();goto fail;}
    if(!ntw16s_global_owner_alive(m->session) || m->session->phase!=NTW16S_GLOBAL_ACTIVE){error=ERROR_INVALID_STATE;goto fail;}
    m->stop=OpenEventW(SYNCHRONIZE,FALSE,stop_name);m->quit=CreateEventW(NULL,TRUE,FALSE,NULL);
    m->work=CreateEventW(NULL,FALSE,FALSE,NULL);m->changed=CreateEventW(NULL,FALSE,FALSE,NULL);
    if(!m->stop || !m->quit || !m->work || !m->changed){error=GetLastError();goto fail;}
    for(unsigned i=0;i<MANAGER_WORKERS;++i){
        m->ready[i]=CreateEventW(NULL,TRUE,FALSE,NULL);m->contexts[i].manager=m;m->contexts[i].index=i;
        if(!m->ready[i]){error=GetLastError();goto fail;}
        m->workers[i]=CreateThread(NULL,0,worker,&m->contexts[i],0,NULL);if(!m->workers[i]){error=GetLastError();goto fail;}
    }
    if(WaitForMultipleObjects(MANAGER_WORKERS,m->ready,TRUE,3000)!=WAIT_OBJECT_0){error=ERROR_TIMEOUT;goto fail;}
    for(unsigned i=0;i<MANAGER_WORKERS;++i)if(m->worker_errors[i]){error=m->worker_errors[i];goto fail;}
    error=ntw16s_global_discover_start(&m->discover,discovered,m);if(error)goto fail;
    *result=m;return ERROR_SUCCESS;
fail:
    {DWORD cleanup=ntw16s_global_manager_stop(m,3000);if(cleanup)*result=m;return cleanup?cleanup:error;}
}
