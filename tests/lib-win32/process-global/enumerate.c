#include "lib-win32/process-global/enumerate.h"
#include "lib-win32/process-global/queue.h"
#include "lib-win32/process-global/registry.h"
#include <stdio.h>
typedef struct observed {DWORD pid,thread;unsigned count,found;BOOL stop;} observed;
static BOOL observe(DWORD pid,DWORD thread,void *context)
{
    observed *o=(observed *)context;
    if(pid!=o->pid)return FALSE;
    ++o->count;if(thread==o->thread)++o->found;
    return !o->stop;
}
typedef struct selection {DWORD pid,chosen,status;unsigned inspected,selected;BOOL tracked;} selection;
static BOOL has_target(DWORD pid,void *context)
{selection *s=context;return pid!=s->pid || s->tracked;}
static DWORD inspect_target(DWORD pid,DWORD tid,void *context)
{selection *s=context;if(pid!=s->pid || !tid)return ERROR_INVALID_PARAMETER;++s->inspected;return s->status;}
static void select_target(DWORD pid,DWORD tid,void *context)
{selection *s=context;if(pid==s->pid && tid){s->chosen=tid;++s->selected;s->tracked=TRUE;}}
static DWORD prefer_ready(DWORD pid,DWORD tid,void *context)
{selection *s=context;if(pid!=s->pid || !tid)return ERROR_INVALID_PARAMETER;return ++s->inspected==1?ERROR_NOT_READY:ERROR_SUCCESS;}
static DWORD WINAPI waiting_thread(void *event){WaitForSingleObject((HANDLE)event,3000);return 0;}
static BOOL remove_marked(void *owner,void *context)
{(void)context;return *(int *)owner!=0;}
static void count_removed(void *owner,void *context)
{(void)owner;++*(unsigned *)context;}
int main(void)
{
    win32_process_global_queue q={0};win32_process_global_queue_node a={0},b={0};
    if(win32_process_global_queue_pop(&q))return 7;
    win32_process_global_queue_push(&q,&a);win32_process_global_queue_push(&q,&b);
    if(win32_process_global_queue_pop(&q)!=&a || win32_process_global_queue_pop(&q)!=&b || win32_process_global_queue_pop(&q) || q.tail)return 8;
    win32_process_global_queue_push(&q,&a);if(win32_process_global_queue_pop(&q)!=&a || q.head || q.tail)return 9;
    win32_process_global_registry_node *slots[2]={0},n1={0},n2={0};
    win32_process_global_registry registry;int keep=0,drop=1;unsigned removed=0;
    win32_process_global_registry_init(&registry,slots,2);
    win32_process_global_registry_insert(&registry,&n1,1,1,&keep);
    win32_process_global_registry_insert(&registry,&n2,3,1,&drop);
    if(win32_process_global_registry_find(&registry,1,1)!=&keep || win32_process_global_registry_find(&registry,3,1)!=&drop || win32_process_global_registry_find(&registry,2,1))return 10;
    if(win32_process_global_registry_reap(&registry,1,remove_marked,count_removed,&removed)!=1 || removed!=1 || win32_process_global_registry_find(&registry,3,1) || win32_process_global_registry_find(&registry,1,1)!=&keep)return 11;
    keep=1;if(win32_process_global_registry_reap(&registry,2,remove_marked,count_removed,&removed)!=1 || removed!=2)return 12;
    observed o={GetCurrentProcessId(),GetCurrentThreadId(),0,0,FALSE};
    DWORD before=0,after=0;
    if(win32_process_global_threads(0,NULL,NULL)!=ERROR_INVALID_PARAMETER)return 1;
    if(!GetProcessHandleCount(GetCurrentProcess(),&before))return 2;
    if(win32_process_global_threads(o.pid,observe,&o) || !o.count || o.found!=1)return 3;
    o.count=0;o.stop=TRUE;
    if(win32_process_global_threads(o.pid,observe,&o) || o.count!=1)return 4;
    o.count=0;o.stop=FALSE;
    if(win32_process_global_threads(0xffffffffu,observe,&o) || o.count)return 5;
    selection sel={.pid=GetCurrentProcessId()};
    if(win32_process_global_scan_existing(NULL,inspect_target,select_target,&sel)!=ERROR_INVALID_PARAMETER ||
       win32_process_global_scan_process(0,inspect_target,select_target,&sel)!=ERROR_INVALID_PARAMETER)return 13;
    if(win32_process_global_scan_existing(has_target,inspect_target,select_target,&sel) || sel.selected!=1 || !sel.chosen)return 14;
    unsigned inspected=sel.inspected;
    if(win32_process_global_scan_existing(has_target,inspect_target,select_target,&sel) || sel.inspected!=inspected || sel.selected!=1)return 15;
    sel=(selection){.pid=GetCurrentProcessId(),.status=ERROR_NOT_READY};
    if(win32_process_global_scan_process(sel.pid,inspect_target,select_target,&sel) || sel.selected!=1 || !sel.chosen)return 16;
    sel=(selection){.pid=GetCurrentProcessId(),.status=ERROR_ACCESS_DENIED};
    if(win32_process_global_scan_process(sel.pid,inspect_target,select_target,&sel) || sel.selected)return 17;
    sel=(selection){.pid=GetCurrentProcessId()};
    if(win32_process_global_scan_process(sel.pid,inspect_target,select_target,&sel) || sel.selected!=1 || sel.inspected!=1)return 18;
    HANDLE event=CreateEventW(NULL,TRUE,FALSE,NULL);if(!event)return 19;
    HANDLE thread=CreateThread(NULL,0,waiting_thread,event,0,NULL);if(!thread){CloseHandle(event);return 20;}
    sel=(selection){.pid=GetCurrentProcessId()};
    DWORD scan=win32_process_global_scan_process(sel.pid,prefer_ready,select_target,&sel);
    SetEvent(event);WaitForSingleObject(thread,3000);CloseHandle(thread);CloseHandle(event);
    if(scan || sel.selected!=1 || sel.inspected!=2)return 21;
    if(!GetProcessHandleCount(GetCurrentProcess(),&after) || after!=before)return 6;
    puts("PROCESS_GLOBAL_PASS own=1 filter=1 early_stop=1 no_match=1 selection=1 fallback=1 prefer_ready=1 policy=1 cleanup=1");return 0;
}
