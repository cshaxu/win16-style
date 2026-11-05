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
    if(!GetProcessHandleCount(GetCurrentProcess(),&after) || after!=before)return 6;
    puts("PROCESS_GLOBAL_PASS own=1 filter=1 early_stop=1 no_match=1 cleanup=1");return 0;
}
