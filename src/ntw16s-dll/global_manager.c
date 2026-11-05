#include "global_manager.h"
#include "global_attach.h"
#include "global_discover.h"
#include "global_session.h"
#include "global_process.h"
#include "lib-win32/process-global/enumerate.h"
#include "lib-win32/process-global/queue.h"
#include "lib-win32/process-global/engine.h"
#include "lib-win32/process-global/registry.h"
#include "lib-win32/process-global/targets.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <tlhelp32.h>

#define MANAGER_WORKERS 4u
#define MANAGER_BUCKETS WIN32_PROCESS_GLOBAL_TARGET_BUCKETS
typedef struct manager_entry {
    win32_process_global_target target;
    ntw16s_global_attach attach;
} manager_entry;
struct ntw16s_global_manager {
    ntw16s_global_discover discover;ntw16s_global_policy policy;ntw16s_global_session *session;
    HANDLE mapping,stop,quit,work,changed;
    win32_process_global_engine engine;
    HDESK desktop;WCHAR *dll_path;WORD machine;DWORD owner_thread;
    ntw16s_global_process *process_monitor;
};

static BOOL expired(win32_process_global_target *target,void *context)
{
    manager_entry *e=(manager_entry *)target;BOOL dead=FALSE;(void)context;
    if(target->state==WIN32_TARGET_ACTIVE && e->attach.start.thread &&
       WaitForSingleObject(e->attach.start.thread,0)==WAIT_OBJECT_0)dead=TRUE;
    else if(target->state==WIN32_TARGET_FAILED){
        if(e->attach.process)dead=WaitForSingleObject(e->attach.process,0)==WAIT_OBJECT_0;
        else{
            HANDLE process=OpenProcess(SYNCHRONIZE,FALSE,target->pid);
            if(process){dead=WaitForSingleObject(process,0)==WAIT_OBJECT_0;CloseHandle(process);}
            else dead=GetLastError()==ERROR_INVALID_PARAMETER;
        }
    }
    return dead && !ntw16s_global_attach_close(&e->attach);
}
static BOOL active_finished(win32_process_global_target *target,void *context)
{manager_entry *e=(manager_entry *)target;(void)context;return e->attach.start.thread && WaitForSingleObject(e->attach.start.thread,0)==WAIT_OBJECT_0;}
static BOOL failed_retryable(win32_process_global_target *target,void *context)
{(void)context;return !((manager_entry *)target)->attach.process;}
static void dispose_target(win32_process_global_target *target,void *context)
{(void)target;(void)context;}
static void discovered(HWND window,DWORD pid,DWORD thread_id,void *context)
{
    ntw16s_global_manager *m=(ntw16s_global_manager *)context;
    if(m->session->phase!=NTW16S_GLOBAL_ACTIVE || pid==m->session->owner_pid || pid==m->session->host_pid)return;
    win32_process_global_engine_submit(&m->engine,pid,thread_id,window);
}
/* Snapshot and per-process scan order live in the public component; identity,
 * session and desktop eligibility remain product policy and are revalidated. */
static BOOL existing_has_process(DWORD pid,void *context)
{
    ntw16s_global_manager *m=context;
    return !pid || pid==GetCurrentProcessId() || pid==m->session->owner_pid || pid==m->session->host_pid ||
        win32_process_global_engine_has_process(&m->engine,pid);
}
static DWORD inspect_existing_thread(DWORD pid,DWORD tid,void *context)
{
    ntw16s_global_manager *m=context;ntw16s_global_identity identity;
    if(!tid || !GetThreadDesktop(tid))return ERROR_NOT_READY;
    return ntw16s_global_policy_inspect_thread(&m->policy,pid,tid,m->machine,&identity);
}
static DWORD inspect_new_thread(DWORD pid,DWORD tid,void *context)
{
    ntw16s_global_manager *m=context;ntw16s_global_identity identity;
    return ntw16s_global_policy_inspect_thread(&m->policy,pid,tid,m->machine,&identity);
}
static void select_thread(DWORD pid,DWORD tid,void *context)
{discovered(NULL,pid,tid,context);}
static void discover_existing_processes(ntw16s_global_manager *m)
{(void)win32_process_global_scan_existing(existing_has_process,inspect_existing_thread,select_thread,m);}
static void discovered_process(DWORD pid,void *context)
{
    ntw16s_global_manager *m=context;DWORD session=0;
    if(!pid || pid==GetCurrentProcessId() || pid==m->session->owner_pid || pid==m->session->host_pid ||
       !ProcessIdToSessionId(pid,&session) || session!=m->policy.session)return;
    (void)win32_process_global_scan_process(pid,inspect_new_thread,select_thread,m);
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
    if(GetWindowThreadProcessId(window,&pid)==scan->entry->target.thread_id && pid==scan->entry->target.pid && IsWindowVisible(window) &&
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
        error=entry->target.window?ntw16s_global_attach_begin(&m->policy,entry->target.window,entry->target.pid,entry->target.thread_id,m->machine,
                                                m->session->cookie,m->dll_path,&entry->attach):
            ntw16s_global_attach_begin_thread(&m->policy,entry->target.pid,entry->target.thread_id,m->machine,
                                             m->session->cookie,m->dll_path,&entry->attach);
        if(error!=ERROR_NOT_READY || entry->target.window || GetTickCount64()>=desktop_deadline)break;
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
    scan_state scan={m,entry,GetTickCount64()+1000};EnumThreadWindows(entry->target.thread_id,notify_top,(LPARAM)&scan);
    return error;
}
static DWORD execute_target(win32_process_global_target *target,void *context,BOOL *retained)
{
    ntw16s_global_manager *m=(ntw16s_global_manager *)context;manager_entry *entry=(manager_entry *)target;
    DWORD error=process_entry(m,entry);
    *retained=error==ERROR_NOT_READY && entry->attach.control_error==ERROR_NOT_READY &&
        entry->attach.start.thread && WaitForSingleObject(entry->attach.start.thread,0)==WAIT_TIMEOUT;
    if(error && !*retained)(void)ntw16s_global_attach_close(&entry->attach);
    return error;
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
    return win32_process_global_engine_wait_idle(&m->engine,timeout);
}
BOOL ntw16s_global_manager_tracks(ntw16s_global_manager *m,DWORD pid,DWORD thread_id)
{
    if(!m || !pid || !thread_id)return FALSE;
    return win32_process_global_engine_contains(&m->engine,pid,thread_id);
}
void ntw16s_global_manager_stats_read(ntw16s_global_manager *m,ntw16s_global_manager_stats *s)
{
    if(!s)return;
    ZeroMemory(s,sizeof(*s));
    if(!m)return;
    win32_process_global_engine_stats stats;win32_process_global_engine_stats_read(&m->engine,&stats);
    s->queued=stats.queued;s->active=stats.active;s->failed=stats.failed;s->overflow=stats.overflow;
}
static DWORD close_target(win32_process_global_target *target,void *context,ULONGLONG deadline)
{
    manager_entry *e=(manager_entry *)target;(void)context;
    if(e->attach.start.thread){ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;WaitForSingleObject(e->attach.start.thread,left);}
    if(e->attach.load.thread){ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;WaitForSingleObject(e->attach.load.thread,left);}
    return ntw16s_global_attach_close(&e->attach);
}
DWORD ntw16s_global_manager_stop(ntw16s_global_manager *m,DWORD timeout)
{
    if(!m || m->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    if(m->process_monitor){DWORD error=ntw16s_global_process_stop(m->process_monitor);if(error)return error;m->process_monitor=NULL;}
    if(m->discover.hook){DWORD error=ntw16s_global_discover_stop(&m->discover);if(error)return error;}
    DWORD close_error=win32_process_global_engine_stop(&m->engine,timeout,close_target,m);if(close_error)return close_error;
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
    const win32_process_global_target_ops ops={expired,active_finished,failed_retryable,execute_target,dispose_target};
    error=win32_process_global_engine_start(&m->engine,m->desktop,m->quit,m->stop,m->work,m->changed,
        MANAGER_WORKERS,sizeof(manager_entry),&ops,m,3000);
    if(error)goto fail;
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
