#include "global_manager.h"
#include "global_attach.h"
#include "global_discover.h"
#include "global_session.h"
#include "global_process.h"
#include "lib-win32/process-global/enumerate.h"
#include "lib-win32/process-global/queue.h"
#include "lib-win32/process-global/dispatch.h"
#include "lib-win32/process-global/registry.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <tlhelp32.h>

#define MANAGER_WORKERS 4u
#define MANAGER_BUCKETS 257u
#define MANAGER_LIMIT 16384u
typedef enum entry_state {ENTRY_QUEUED,ENTRY_WORKING,ENTRY_ACTIVE,ENTRY_FAILED} entry_state;
typedef struct manager_entry {
    win32_process_global_queue_node queue_node;
    DWORD pid,thread_id;HWND window;entry_state state;unsigned attempts;
    ntw16s_global_attach attach;
    win32_process_global_registry_node registry_node;
} manager_entry;
struct ntw16s_global_manager {
    ntw16s_global_discover discover;ntw16s_global_policy policy;ntw16s_global_session *session;
    HANDLE mapping,stop,quit,work,changed,workers[MANAGER_WORKERS],ready[MANAGER_WORKERS];
    win32_process_global_dispatch contexts[MANAGER_WORKERS];
    HDESK desktop;WCHAR *dll_path;WORD machine;DWORD owner_thread;SRWLOCK lock;
    win32_process_global_registry_node *buckets[MANAGER_BUCKETS];win32_process_global_registry registry;win32_process_global_queue queue;unsigned count;
    volatile LONG queued,working,active,failed,overflow;
    SRWLOCK process_locks[64];
    ntw16s_global_process *process_monitor;
};

static void notify_change(ntw16s_global_manager *m){SetEvent(m->changed);}
/* The registry owns only its intrusive index; this callback applies the
 * product's existing ticket-lifetime rules under the manager lock. */
static BOOL expired(void *owner,void *context)
{
    manager_entry *e=(manager_entry *)owner;BOOL dead=FALSE;(void)context;
    if(e->state==ENTRY_ACTIVE && e->attach.start.thread &&
       WaitForSingleObject(e->attach.start.thread,0)==WAIT_OBJECT_0)dead=TRUE;
    else if(e->state==ENTRY_FAILED){
        if(e->attach.process)dead=WaitForSingleObject(e->attach.process,0)==WAIT_OBJECT_0;
        else{
            HANDLE process=OpenProcess(SYNCHRONIZE,FALSE,e->pid);
            if(process){dead=WaitForSingleObject(process,0)==WAIT_OBJECT_0;CloseHandle(process);}
            else dead=GetLastError()==ERROR_INVALID_PARAMETER;
        }
    }
    return dead && !ntw16s_global_attach_close(&e->attach);
}
static void release_entry(void *owner,void *context)
{
    manager_entry *e=(manager_entry *)owner;ntw16s_global_manager *m=(ntw16s_global_manager *)context;
    if(e->state==ENTRY_ACTIVE)InterlockedDecrement(&m->active);
    --m->count;free(e);
}
static void reap(ntw16s_global_manager *m,unsigned budget)
{if(win32_process_global_registry_reap(&m->registry,budget,expired,release_entry,m))notify_change(m);}
static void enqueue(ntw16s_global_manager *m,manager_entry *entry)
{
    entry->state=ENTRY_QUEUED;win32_process_global_queue_push(&m->queue,&entry->queue_node);
    InterlockedIncrement(&m->queued);SetEvent(m->work);
}
static void discovered(HWND window,DWORD pid,DWORD thread_id,void *context)
{
    ntw16s_global_manager *m=(ntw16s_global_manager *)context;
    if(m->session->phase!=NTW16S_GLOBAL_ACTIVE || pid==m->session->owner_pid || pid==m->session->host_pid)return;
    AcquireSRWLockExclusive(&m->lock);
    reap(m,m->count>=MANAGER_LIMIT?MANAGER_BUCKETS:8u);
    manager_entry *entry=(manager_entry *)win32_process_global_registry_find(&m->registry,pid,thread_id);
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
    win32_process_global_registry_insert(&m->registry,&entry->registry_node,pid,thread_id,entry);++m->count;enqueue(m,entry);
    ReleaseSRWLockExclusive(&m->lock);
}
/* Startup-only snapshot. Runtime discovery remains event driven. Prefer the
 * existing window entry for a process and add one eligible thread only when
 * no window entry is already queued. Identity is checked again by the worker. */
static BOOL discover_existing_thread(DWORD pid,DWORD tid,void *context)
{
    ntw16s_global_manager *m=(ntw16s_global_manager *)context;BOOL found=FALSE;
    if(!pid || !tid || pid==GetCurrentProcessId() || pid==m->session->owner_pid || pid==m->session->host_pid)return TRUE;
    if(!GetThreadDesktop(tid))return TRUE;
    AcquireSRWLockShared(&m->lock);
    for(unsigned i=0;i<MANAGER_BUCKETS && !found;++i)
        for(win32_process_global_registry_node *node=m->buckets[i];node;node=node->next){
            manager_entry *entry=(manager_entry *)node->owner;
            if(entry->pid==pid && entry->state!=ENTRY_FAILED){found=TRUE;break;}
        }
    ReleaseSRWLockShared(&m->lock);
    if(!found){
        ntw16s_global_identity identity;
        if(!ntw16s_global_policy_inspect_thread(&m->policy,pid,tid,m->machine,&identity))discovered(NULL,pid,tid,m);
    }
    return TRUE;
}
static void discover_existing_processes(ntw16s_global_manager *m)
{(void)win32_process_global_threads(0,discover_existing_thread,m);}
/* Only inspect threads belonging to the reported PID. The worker repeats all
 * identity and desktop checks before modifying the target. */
typedef struct discover_process_scan {ntw16s_global_manager *manager;DWORD pending;BOOL queued;} discover_process_scan;
static BOOL discover_process_thread(DWORD pid,DWORD tid,void *context)
{
    discover_process_scan *scan=(discover_process_scan *)context;ntw16s_global_identity identity;
    DWORD check=ntw16s_global_policy_inspect_thread(&scan->manager->policy,pid,tid,scan->manager->machine,&identity);
    if(!check){discovered(NULL,pid,tid,scan->manager);scan->queued=TRUE;return FALSE;}
    if(check==ERROR_NOT_READY && !scan->pending)scan->pending=tid;
    return TRUE;
}
static void discovered_process(DWORD pid,void *context)
{
    ntw16s_global_manager *m=(ntw16s_global_manager *)context;DWORD session=0;
    if(!pid || pid==GetCurrentProcessId() || pid==m->session->owner_pid || pid==m->session->host_pid ||
       !ProcessIdToSessionId(pid,&session) || session!=m->policy.session)return;
    discover_process_scan scan={m,0,FALSE};
    (void)win32_process_global_threads(pid,discover_process_thread,&scan);
    if(!scan.queued && scan.pending)discovered(NULL,pid,scan.pending,m);
}
#ifdef NTW16S_GLOBAL_MANAGER_TEST
void ntw16s_global_manager_test_process(ntw16s_global_manager *manager,DWORD pid)
{discovered_process(pid,manager);}
#endif
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
    DWORD error;ULONGLONG desktop_deadline=GetTickCount64()+1000;
    for(;;){
        if(m->session->phase!=NTW16S_GLOBAL_ACTIVE){error=ERROR_CANCELLED;break;}
        error=entry->window?ntw16s_global_attach_begin(&m->policy,entry->window,entry->pid,entry->thread_id,m->machine,
                                                m->session->cookie,m->dll_path,&entry->attach):
            ntw16s_global_attach_begin_thread(&m->policy,entry->pid,entry->thread_id,m->machine,
                                             m->session->cookie,m->dll_path,&entry->attach);
        if(error!=ERROR_NOT_READY || entry->window || GetTickCount64()>=desktop_deadline)break;
        HANDLE waits[]={m->quit,m->stop};DWORD wait=WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,20);
        if(wait!=WAIT_TIMEOUT){error=wait==WAIT_FAILED?GetLastError():ERROR_CANCELLED;break;}
    }
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
    while(error==ERROR_TIMEOUT && entry->attach.control_error==ERROR_TIMEOUT && GetTickCount64()<deadline){
        if(WaitForSingleObject(m->quit,0)==WAIT_OBJECT_0 || WaitForSingleObject(m->stop,0)==WAIT_OBJECT_0 ||
           m->session->phase!=NTW16S_GLOBAL_ACTIVE)return ERROR_CANCELLED;
        error=ntw16s_global_attach_apply(&entry->attach);
    }
    if(error && error!=ERROR_NOT_READY)return error;
    scan_state scan={m,entry,GetTickCount64()+1000};EnumThreadWindows(entry->thread_id,notify_top,(LPARAM)&scan);
    return error;
}
static BOOL process_next(void *parameter)
{
    ntw16s_global_manager *m=(ntw16s_global_manager *)parameter;
    AcquireSRWLockExclusive(&m->lock);manager_entry *entry=(manager_entry *)win32_process_global_queue_pop(&m->queue);
    if(entry){entry->state=ENTRY_WORKING;InterlockedDecrement(&m->queued);InterlockedIncrement(&m->working);}
    ReleaseSRWLockExclusive(&m->lock);if(!entry)return FALSE;
    /* Late patching snapshots all threads in a target process. Do not
     * create competing remote workers for two of its GUI threads. */
    SRWLOCK *process_lock=&m->process_locks[entry->pid%ARRAYSIZE(m->process_locks)];
    AcquireSRWLockExclusive(process_lock);
    DWORD error=process_entry(m,entry);
    ReleaseSRWLockExclusive(process_lock);
    BOOL installed=error==ERROR_NOT_READY && entry->attach.control_error==ERROR_NOT_READY &&
        entry->attach.start.thread && WaitForSingleObject(entry->attach.start.thread,0)==WAIT_TIMEOUT;
    if(error && !installed)(void)ntw16s_global_attach_close(&entry->attach);
    AcquireSRWLockExclusive(&m->lock);++entry->attempts;
    InterlockedDecrement(&m->working);
    if(!error || installed){entry->state=ENTRY_ACTIVE;InterlockedIncrement(&m->active);if(error)InterlockedIncrement(&m->failed);}
    else{
        entry->state=ENTRY_FAILED;InterlockedIncrement(&m->failed);
    }
    ReleaseSRWLockExclusive(&m->lock);notify_change(m);
    return TRUE;
}
HANDLE ntw16s_global_manager_changed(ntw16s_global_manager *m){return m?m->changed:NULL;}
DWORD ntw16s_global_manager_refresh(ntw16s_global_manager *m)
{
    if(!m || m->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    return ntw16s_global_discover_refresh(&m->discover);
}
DWORD ntw16s_global_manager_wait_idle(ntw16s_global_manager *m,DWORD timeout)
{
    if(!m || m->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    ULONGLONG deadline=GetTickCount64()+timeout;HANDLE waits[]={m->stop,m->quit,m->changed};
    for(;;){
        AcquireSRWLockShared(&m->lock);BOOL idle=m->queued==0 && m->working==0;ReleaseSRWLockShared(&m->lock);
        if(idle)return ERROR_SUCCESS;
        ULONGLONG now=GetTickCount64();if(now>=deadline)return ERROR_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,(DWORD)(deadline-now),QS_ALLINPUT);
        if(result==WAIT_OBJECT_0 || result==WAIT_OBJECT_0+1)return ERROR_CANCELLED;
        if(result==WAIT_TIMEOUT)return ERROR_TIMEOUT;
        if(result==WAIT_FAILED)return GetLastError();
        if(result==WAIT_OBJECT_0+ARRAYSIZE(waits)){MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}}
        else if(result!=WAIT_OBJECT_0+2)return ERROR_INVALID_STATE;
    }
}
BOOL ntw16s_global_manager_tracks(ntw16s_global_manager *m,DWORD pid,DWORD thread_id)
{
    if(!m || !pid || !thread_id)return FALSE;
    AcquireSRWLockShared(&m->lock);manager_entry *e=(manager_entry *)win32_process_global_registry_find(&m->registry,pid,thread_id);
    BOOL found=e!=NULL;ReleaseSRWLockShared(&m->lock);return found;
}
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
    if(m->process_monitor){DWORD error=ntw16s_global_process_stop(m->process_monitor);if(error)return error;m->process_monitor=NULL;}
    if(m->discover.hook){DWORD error=ntw16s_global_discover_stop(&m->discover);if(error)return error;}
    SetEvent(m->quit);ULONGLONG deadline=GetTickCount64()+timeout;
    for(unsigned i=0;i<MANAGER_WORKERS;++i)if(m->workers[i]){
        ULONGLONG now=GetTickCount64();DWORD remaining=now<deadline?(DWORD)(deadline-now):0;
        DWORD wait=WaitForSingleObject(m->workers[i],remaining);
        if(wait!=WAIT_OBJECT_0)return wait==WAIT_TIMEOUT?ERROR_TIMEOUT:(wait==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE);
    }
    for(unsigned i=0;i<MANAGER_BUCKETS;++i)for(win32_process_global_registry_node *node=m->buckets[i];node;node=node->next){
        manager_entry *e=(manager_entry *)node->owner;
        if(e->attach.start.thread){ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;WaitForSingleObject(e->attach.start.thread,left);}
        if(e->attach.load.thread){ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;WaitForSingleObject(e->attach.load.thread,left);}
        DWORD error=ntw16s_global_attach_close(&e->attach);if(error)return error;
    }
    for(unsigned i=0;i<MANAGER_BUCKETS;++i){win32_process_global_registry_node *node=m->buckets[i];while(node){win32_process_global_registry_node *next=node->next;free(node->owner);node=next;}}
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
    win32_process_global_registry_init(&m->registry,m->buckets,MANAGER_BUCKETS);
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
        m->ready[i]=CreateEventW(NULL,TRUE,FALSE,NULL);
        if(!m->ready[i]){error=GetLastError();goto fail;}
        m->contexts[i]=(win32_process_global_dispatch){m->desktop,m->quit,m->stop,m->work,m->ready[i],0,process_next,m};
        m->workers[i]=CreateThread(NULL,0,win32_process_global_dispatch_run,&m->contexts[i],0,NULL);if(!m->workers[i]){error=GetLastError();goto fail;}
    }
    if(WaitForMultipleObjects(MANAGER_WORKERS,m->ready,TRUE,3000)!=WAIT_OBJECT_0){error=ERROR_TIMEOUT;goto fail;}
    for(unsigned i=0;i<MANAGER_WORKERS;++i)if(m->contexts[i].error){error=m->contexts[i].error;goto fail;}
    error=ntw16s_global_discover_start(&m->discover,discovered,m);if(error)goto fail;
    /* The optional system process trace requires elevation. Retain normal
     * window-event discovery when that permission is not available. */
    error=ntw16s_global_process_start(discovered_process,m,&m->process_monitor);
    if(error && m->process_monitor)goto fail;
    if(error)OutputDebugStringW(L"run16s: global process events unavailable; using window discovery\n");
    discover_existing_processes(m);
    /* Targets can appear while the initial snapshot is inspected and before
     * this owner thread begins dispatching queued accessibility events. */
    error=ntw16s_global_discover_refresh(&m->discover);if(error)goto fail;
    *result=m;return ERROR_SUCCESS;
fail:
    {DWORD cleanup=ntw16s_global_manager_stop(m,3000);if(cleanup)*result=m;return cleanup?cleanup:error;}
}
