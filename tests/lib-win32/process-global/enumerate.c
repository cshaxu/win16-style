#include "lib-win32/process-global/enumerate.h"
#include <stdio.h>
typedef struct observed {DWORD pid,thread;unsigned count,found;BOOL stop;} observed;
static BOOL observe(DWORD pid,DWORD thread,void *context)
{
    observed *o=(observed *)context;
    if(pid!=o->pid)return FALSE;
    ++o->count;if(thread==o->thread)++o->found;
    return !o->stop;
}
int main(void)
{
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
