#include "manager.h"
#include "trace.h"
#include "lib-win32-style/application/adapters/uwp/target.h"
#ifdef STYLE_GLOBAL_MANAGER_TEST
#include <stdio.h>
static DWORD test_pid,test_second_pid;
void style_global_manager_test_filter(DWORD pid){test_pid=pid;test_second_pid=0;}
void style_global_manager_test_second(DWORD pid){test_second_pid=pid;}
static BOOL test_allowed(DWORD pid){return pid==test_pid||(test_second_pid&&pid==test_second_pid);}
#endif
#include "lib-win32/process-global/enumerate.h"
#include "lib-win32/process-global/window.h"
#include "lib-win32/process-global/transaction.h"
#include "lib-win32/process-global/engine.h"
#include "lib-win32/process-global/targets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>

#define MANAGER_WORKERS 4u
typedef struct manager_entry {
    win32_process_global_target target;
    style_global_attach attach;
} manager_entry;
struct style_global_manager {
    const style_global_manager_config *config;
    win32_process_global_window discover;win32_process_global_policy policy;style_global_session *session;
    HANDLE mapping,stop,quit,work,changed;
    win32_process_global_engine engine;
    WCHAR *dll_path;WORD machine;DWORD owner_thread;
    win32_process_global_monitor *process_monitor;
    style_uwp_global *uwp;
};

static void report(style_global_manager *m,style_global_report detail)
{if(m->config && m->config->report)m->config->report(&detail,m->config->context);}
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
    return dead && !style_global_attach_close(&e->attach);
}
static BOOL active_finished(win32_process_global_target *target,void *context)
{manager_entry *e=(manager_entry *)target;(void)context;return e->attach.start.thread && WaitForSingleObject(e->attach.start.thread,0)==WAIT_OBJECT_0;}
static BOOL failed_retryable(win32_process_global_target *target,void *context)
{(void)context;return !((manager_entry *)target)->attach.process;}
static void dispose_target(win32_process_global_target *target,void *context)
{(void)target;(void)context;}
static void discovered(HWND window,DWORD pid,DWORD thread_id,void *context)
{
    style_global_manager *m=(style_global_manager *)context;
    if(m->session->phase!=STYLE_GLOBAL_ACTIVE || pid==m->session->owner_pid || pid==m->session->host_pid)return;
#ifdef STYLE_GLOBAL_MANAGER_TEST
    if(test_pid&&!test_allowed(pid)){style_uwp_target target;
        if(!window||!style_uwp_target_window(window,&target))return;
        if(!test_allowed(target.pid)){printf("UWP_TEST_FILTER skipped_pid=%lu allowed=%lu\n",target.pid,test_pid);return;}}
#endif
    if(window&&style_uwp_global_observe(m->uwp,window))return;
    win32_process_global_engine_submit(&m->engine,pid,thread_id,window);
}
/* Snapshot and per-process scan order live in the public component; identity,
 * session and desktop eligibility remain product policy and are revalidated. */
static BOOL existing_has_process(DWORD pid,void *context)
{
    style_global_manager *m=context;
#ifdef STYLE_GLOBAL_MANAGER_TEST
    if(test_pid&&!test_allowed(pid))return TRUE;
#endif
    return !pid || pid==GetCurrentProcessId() || pid==m->session->owner_pid || pid==m->session->host_pid ||
        win32_process_global_engine_has_process(&m->engine,pid);
}
static DWORD inspect_existing_thread(DWORD pid,DWORD tid,void *context)
{
    style_global_manager *m=context;win32_process_global_identity identity;
    if(!tid || !GetThreadDesktop(tid))return ERROR_NOT_READY;
    return win32_process_global_policy_inspect_thread(&m->policy,pid,tid,m->machine,&identity,m->config->attach->allowed);
}
static DWORD inspect_new_thread(DWORD pid,DWORD tid,void *context)
{
    style_global_manager *m=context;win32_process_global_identity identity;
    return win32_process_global_policy_inspect_thread(&m->policy,pid,tid,m->machine,&identity,m->config->attach->allowed);
}
static void select_thread(DWORD pid,DWORD tid,void *context)
{discovered(NULL,pid,tid,context);}
static void discovered_process(DWORD pid,void *context)
{
    style_global_manager *m=context;DWORD session=0;
    if(!pid || pid==GetCurrentProcessId() || pid==m->session->owner_pid || pid==m->session->host_pid ||
       !ProcessIdToSessionId(pid,&session) || session!=m->policy.session)return;
    DWORD error=win32_process_global_scan_process(pid,inspect_new_thread,select_thread,m);
    if(error)report(m,(style_global_report){.kind=STYLE_GLOBAL_PROCESS_SCAN,.pid=pid,.error=error});
}
#ifdef STYLE_GLOBAL_MANAGER_TEST
void style_global_manager_test_process(style_global_manager *manager,DWORD pid)
{discovered_process(pid,manager);}
#endif
typedef struct scan_state {style_global_manager *manager;manager_entry *entry;} scan_state;
static BOOL notify_window(HWND window,void *parameter)
{
    scan_state *scan=(scan_state *)parameter;DWORD pid=0;
    if(GetWindowThreadProcessId(window,&pid)==scan->entry->target.thread_id && pid==scan->entry->target.pid && IsWindowVisible(window) &&
       !win32_process_global_policy_revalidate(scan->entry->attach.process,window,&scan->entry->attach.identity)){
        UINT message=RegisterWindowMessageW(scan->manager->config->attach->control);DWORD_PTR ignored=0;
        if(message)SendMessageTimeoutW(window,message,scan->manager->session->cookie,scan->manager->config->attach->apply,
                                      SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,30,&ignored);
    }
    return TRUE;
}
typedef struct process_run {style_global_manager *manager;manager_entry *entry;BOOL begun;} process_run;
static BOOL process_active(void *context)
{return ((process_run *)context)->manager->session->phase==STYLE_GLOBAL_ACTIVE;}
static DWORD process_begin(void *context)
{
    process_run *r=context;style_global_manager *m=r->manager;manager_entry *e=r->entry;
    DWORD error=style_global_attach_begin(m->config->attach,&m->policy,e->target.window,
        e->target.pid,e->target.thread_id,m->machine,m->session->cookie,m->dll_path,&e->attach);
    if(!error)r->begun=TRUE;
    return error;
}
static DWORD process_advance(void *context)
{return style_global_attach_advance(&((process_run *)context)->entry->attach,NULL);}
static unsigned process_pending(void *context,HANDLE handles[2])
{
    style_global_attach *a=&((process_run *)context)->entry->attach;
    if(a->load.thread){handles[0]=a->load.thread;return 1;}
    if(a->start.ready && a->start.thread){handles[0]=a->start.ready;handles[1]=a->start.thread;return 2;}
    return 0;
}
static BOOL process_retry(void *context,DWORD error)
{return error==ERROR_TIMEOUT && ((process_run *)context)->entry->attach.control_error==ERROR_TIMEOUT;}
static DWORD process_apply(void *context)
{return style_global_attach_apply(&((process_run *)context)->entry->attach);}
static DWORD process_entry(style_global_manager *m,manager_entry *entry)
{
    ULONGLONG deadline=GetTickCount64()+5000;
    if(WaitForSingleObject(m->quit,0)==WAIT_OBJECT_0 || WaitForSingleObject(m->stop,0)==WAIT_OBJECT_0 ||
       m->session->phase!=STYLE_GLOBAL_ACTIVE)return ERROR_CANCELLED;
    if(entry->attach.process){DWORD closed=style_global_attach_close(&entry->attach);if(closed)return closed;}
    static const win32_process_global_transaction_run_ops ops={process_active,process_begin,process_advance,process_pending,process_retry,process_apply};
    process_run run={m,entry,FALSE};DWORD error=win32_process_global_transaction_run(m->quit,m->stop,deadline,GetTickCount64()+1000,
        entry->target.window==NULL,&ops,&run);
    if(error && (!run.begun || error!=ERROR_NOT_READY))return error;
    scan_state scan={m,entry};(void)win32_process_global_window_walk(entry->target.thread_id,GetTickCount64()+1000,notify_window,&scan);
    return error;
}
static DWORD execute_target(win32_process_global_target *target,void *context,BOOL *retained)
{
    style_global_manager *m=(style_global_manager *)context;manager_entry *entry=(manager_entry *)target;
    DWORD error=process_entry(m,entry);
#ifdef STYLE_GLOBAL_MANAGER_TEST
    if(error){
        DWORD appcontainer=MAXDWORD,bytes=0;HANDLE token=NULL;
        HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,target->pid);
        if(process&&OpenProcessToken(process,TOKEN_QUERY,&token)){
            if(!GetTokenInformation(token,TokenIsAppContainer,&appcontainer,sizeof(appcontainer),&bytes))appcontainer=MAXDWORD;
            CloseHandle(token);
        }
        if(process)CloseHandle(process);
        printf("CLASSIC_TARGET_RESULT pid=%lu tid=%lu window=%p error=%lu appcontainer=%lu\n",
            target->pid,target->thread_id,(void*)target->window,error,appcontainer);
    }
#endif
    *retained=error==ERROR_NOT_READY && entry->attach.control_error==ERROR_NOT_READY &&
        entry->attach.start.thread && WaitForSingleObject(entry->attach.start.thread,0)==WAIT_TIMEOUT;
    if(error && !*retained)(void)style_global_attach_close(&entry->attach);
    return error;
}
HANDLE style_global_manager_changed(style_global_manager *m){return m?m->changed:NULL;}
DWORD style_global_manager_refresh(style_global_manager *m)
{
    if(!m || m->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    return win32_process_global_window_refresh(&m->discover);
}
DWORD style_global_manager_wait_idle(style_global_manager *m,DWORD timeout)
{
    if(!m || m->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    ULONGLONG deadline=GetTickCount64()+timeout;
    DWORD error=win32_process_global_engine_wait_idle(&m->engine,timeout);
    if(error)return error;
    ULONGLONG now=GetTickCount64();
    return style_uwp_global_wait_idle(m->uwp,now<deadline?(DWORD)(deadline-now):0);
}
BOOL style_global_manager_tracks(style_global_manager *m,DWORD pid,DWORD thread_id)
{
    if(!m || !pid || !thread_id)return FALSE;
    return win32_process_global_engine_contains(&m->engine,pid,thread_id);
}
void style_global_manager_stats_read(style_global_manager *m,style_global_manager_stats *s)
{
    if(!s)return;
    ZeroMemory(s,sizeof(*s));
    if(!m)return;
    win32_process_global_engine_stats stats;win32_process_global_engine_stats_read(&m->engine,&stats);
    s->queued=stats.queued;s->active=stats.active;s->failed=stats.failed;s->overflow=stats.overflow;
    style_uwp_global_stats_read(m->uwp,&s->queued,&s->active,&s->failed);
}
static DWORD close_target(win32_process_global_target *target,void *context,ULONGLONG deadline)
{
    manager_entry *e=(manager_entry *)target;style_global_manager *m=context;
    DWORD worker=WAIT_OBJECT_0,loader=WAIT_OBJECT_0;
    if(e->attach.start.thread){ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;worker=WaitForSingleObject(e->attach.start.thread,left);}
    if(e->attach.load.thread){
        ULONGLONG now=GetTickCount64();DWORD left=worker==WAIT_OBJECT_0?0:now<deadline?(DWORD)(deadline-now):0;
        loader=WaitForSingleObject(e->attach.load.thread,left);
    }
    if(loader==WAIT_TIMEOUT && worker==WAIT_OBJECT_0){
        DWORD abandoned=win32_process_global_load_abandon(&e->attach.load);
        if(!abandoned)report(m,(style_global_report){.kind=STYLE_GLOBAL_LOADER_RETAINED,.pid=target->pid,.thread=target->thread_id});
        else if(abandoned!=ERROR_INVALID_STATE)return abandoned;
    }
    DWORD error=style_global_attach_close(&e->attach);
    if(error)report(m,(style_global_report){.kind=STYLE_GLOBAL_TARGET_STOP,.pid=target->pid,.thread=target->thread_id,
        .error=error,.worker=worker,.loader=loader,.phase=e->attach.phase});
    return error;
}
DWORD style_global_manager_stop(style_global_manager *m,DWORD timeout)
{
    if(!m || m->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    if(m->process_monitor){DWORD error=win32_process_global_monitor_stop(m->process_monitor);if(error){report(m,(style_global_report){.kind=STYLE_GLOBAL_MONITOR_STOP,.error=error});return error;}m->process_monitor=NULL;}
    if(m->discover.hook){DWORD error=win32_process_global_window_stop(&m->discover);if(error){report(m,(style_global_report){.kind=STYLE_GLOBAL_DISCOVERY_STOP,.error=error});return error;}}
    DWORD close_error=win32_process_global_engine_stop(&m->engine,timeout,close_target,m);
    if(close_error){report(m,(style_global_report){.kind=STYLE_GLOBAL_TARGETS_STOP,.error=close_error});return close_error;}
    if(m->uwp){close_error=style_uwp_global_stop(m->uwp,timeout);if(close_error){report(m,(style_global_report){.kind=STYLE_GLOBAL_UWP_STOP,.error=close_error});return close_error;}m->uwp=NULL;}
    if(m->session)UnmapViewOfFile(m->session);
    if(m->mapping)CloseHandle(m->mapping);
    if(m->stop)CloseHandle(m->stop);
    if(m->quit)CloseHandle(m->quit);
    if(m->work)CloseHandle(m->work);
    if(m->changed)CloseHandle(m->changed);
    free(m->dll_path);free(m);return ERROR_SUCCESS;
}
#ifdef STYLE_GLOBAL_MANAGER_TEST
typedef struct test_loader {HANDLE go;const char *path;} test_loader;
static DWORD WINAPI test_loader_wait(void *value)
{
    test_loader *loader=value;
    return WaitForSingleObject(loader->go,3000)!=WAIT_OBJECT_0 || strcmp(loader->path,"retained")?1:0;
}
DWORD style_global_manager_test_abandon(void)
{
    style_global_manager *m=calloc(1,sizeof(*m));manager_entry *entry=calloc(1,sizeof(*entry));
    HANDLE go=CreateEventW(NULL,TRUE,FALSE,NULL),watch=NULL;
    char *memory=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!m || !entry || !go || !memory){free(m);free(entry);if(go)CloseHandle(go);if(memory)VirtualFree(memory,0,MEM_RELEASE);return ERROR_NOT_ENOUGH_MEMORY;}
    strcpy(memory,"retained");test_loader loader={go,memory};
    if(!DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&entry->attach.load.process,0,FALSE,DUPLICATE_SAME_ACCESS))goto setup_failed;
    entry->attach.load.remote_path=memory;watch=CreateThread(NULL,0,test_loader_wait,&loader,0,NULL);
    if(!watch || !DuplicateHandle(GetCurrentProcess(),watch,GetCurrentProcess(),&entry->attach.load.thread,0,FALSE,DUPLICATE_SAME_ACCESS))goto setup_failed;
    m->owner_thread=GetCurrentThreadId();m->engine.owner=m->owner_thread;m->engine.initialized=TRUE;
    m->engine.targets.ops.dispose=dispose_target;
    win32_process_global_registry_init(&m->engine.targets.registry,m->engine.targets.buckets,WIN32_PROCESS_GLOBAL_TARGET_BUCKETS);
    win32_process_global_registry_insert(&m->engine.targets.registry,&entry->target.registry_node,
                                         GetCurrentProcessId(),m->owner_thread,&entry->target);
    m->engine.targets.count=1;
    ULONGLONG before=GetTickCount64();DWORD stopped=style_global_manager_stop(m,2000);
    ULONGLONG elapsed=GetTickCount64()-before;SetEvent(go);
    DWORD wait=WaitForSingleObject(watch,3000),code=1;if(wait==WAIT_OBJECT_0)GetExitCodeThread(watch,&code);
    if(stopped)(void)style_global_manager_stop(m,3000);
    CloseHandle(watch);CloseHandle(go);BOOL freed=VirtualFree(memory,0,MEM_RELEASE);
    return !stopped && elapsed<1000 && wait==WAIT_OBJECT_0 && !code && freed?ERROR_SUCCESS:ERROR_INVALID_STATE;
setup_failed:
    {DWORD error=GetLastError();if(watch){SetEvent(go);WaitForSingleObject(watch,3000);CloseHandle(watch);}
     if(entry->attach.load.process)CloseHandle(entry->attach.load.process);
     CloseHandle(go);VirtualFree(memory,0,MEM_RELEASE);free(entry);free(m);return error?error:ERROR_INVALID_STATE;}
}
#endif
DWORD style_global_manager_start(const style_global_manager_config *config,const WCHAR *dll_path,WORD machine,style_global_manager **result)
{
    WCHAR name[STYLE_GLOBAL_NAME_CAP],stop_name[STYLE_GLOBAL_NAME_CAP+16];DWORD error=ERROR_SUCCESS;
    if(result)*result=NULL;
    if(!dll_path || !*dll_path || !result || machine!=(sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386))return ERROR_INVALID_PARAMETER;
    style_global_manager *m=(style_global_manager *)calloc(1,sizeof(*m));if(!m)return ERROR_NOT_ENOUGH_MEMORY;
    m->config=config;m->owner_thread=GetCurrentThreadId();HDESK desktop=GetThreadDesktop(m->owner_thread);m->machine=machine;
    size_t length=wcslen(dll_path);if(length>=32752){error=ERROR_FILENAME_EXCED_RANGE;goto fail;}
    m->dll_path=(WCHAR *)calloc(length+1,sizeof(WCHAR));if(!m->dll_path){error=ERROR_NOT_ENOUGH_MEMORY;goto fail;}
    memcpy(m->dll_path,dll_path,(length+1)*sizeof(WCHAR));
    if(!desktop || !style_global_session_name(config->attach->start->space,name,ARRAYSIZE(name)) || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0){error=ERROR_INVALID_NAME;goto fail;}
    error=win32_process_global_policy_init(&m->policy);if(error)goto fail;
    m->mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(!m->mapping){error=GetLastError();goto fail;}
    m->session=(style_global_session *)MapViewOfFile(m->mapping,FILE_MAP_READ,0,0,sizeof(*m->session));
    if(!m->session){error=GetLastError();goto fail;}
    if(!style_global_owner_alive(config->attach->start->space,m->session) || m->session->phase!=STYLE_GLOBAL_ACTIVE){error=ERROR_INVALID_STATE;goto fail;}
    m->stop=OpenEventW(SYNCHRONIZE,FALSE,stop_name);if(!m->stop){error=GetLastError();goto fail;}
    m->quit=CreateEventW(NULL,TRUE,FALSE,NULL);if(!m->quit){error=GetLastError();goto fail;}
    m->work=CreateEventW(NULL,FALSE,FALSE,NULL);if(!m->work){error=GetLastError();goto fail;}
    m->changed=CreateEventW(NULL,FALSE,FALSE,NULL);if(!m->changed){error=GetLastError();goto fail;}
    const win32_process_global_target_ops ops={expired,active_finished,failed_retryable,execute_target,dispose_target};
    error=win32_process_global_engine_start(&m->engine,desktop,m->quit,m->stop,m->work,m->changed,
        MANAGER_WORKERS,sizeof(manager_entry),&ops,m,3000);
    if(error)goto fail;
    style_uwp_global_services uwp_services;
    if(config->uwp_services && config->uwp_services(m->session->style,m->session->cookie,machine,&uwp_services,config->context)){
        error=style_uwp_global_start(&m->policy,m->dll_path,desktop,m->quit,m->stop,&uwp_services,&m->uwp);
        if(error)goto fail;
    }
    error=win32_process_global_window_start_ex(&m->discover,discovered,m,
        machine==IMAGE_FILE_MACHINE_AMD64?WIN32_PROCESS_GLOBAL_WINDOW_UNCLOAKED:0);if(error)goto fail;
    /* The optional system process trace requires elevation. Retain normal
     * window-event discovery when that permission is not available. */
    error=style_global_trace_start(config->trace_prefix,discovered_process,m,&m->process_monitor);
    if(error && m->process_monitor)goto fail;
    if(error)report(m,(style_global_report){.kind=STYLE_GLOBAL_TRACE_UNAVAILABLE,.error=error});
    error=win32_process_global_scan_existing(existing_has_process,inspect_existing_thread,select_thread,m);if(error)goto fail;
    /* Targets can appear while the initial snapshot is inspected and before
     * this owner thread begins dispatching queued accessibility events. */
    error=win32_process_global_window_refresh(&m->discover);if(error)goto fail;
    *result=m;return ERROR_SUCCESS;
fail:
    {DWORD cleanup=style_global_manager_stop(m,3000);if(cleanup)*result=m;return cleanup?cleanup:error;}
}
