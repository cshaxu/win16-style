#include "lib-win32/process-global/dispatch.h"
#include <stdio.h>
typedef struct state {volatile LONG pending,processed;HANDLE done;} state;
static BOOL next(void *parameter)
{
    state *s=(state *)parameter;
    if(InterlockedDecrement(&s->pending)<0){InterlockedIncrement(&s->pending);return FALSE;}
    if(InterlockedIncrement(&s->processed)==2)SetEvent(s->done);
    return TRUE;
}
typedef struct blocked_state {HANDLE entered, release;} blocked_state;
static BOOL blocked(void *parameter)
{
    blocked_state *s=(blocked_state *)parameter;
    SetEvent(s->entered);WaitForSingleObject(s->release,3000);return FALSE;
}
int main(void)
{
    HANDLE quit=CreateEventW(NULL,TRUE,FALSE,NULL),stop=CreateEventW(NULL,TRUE,FALSE,NULL);
    HANDLE work=CreateEventW(NULL,FALSE,FALSE,NULL),ready=CreateEventW(NULL,TRUE,FALSE,NULL),done=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!quit || !stop || !work || !ready || !done)return 1;
    state s={2,0,done};win32_process_global_dispatch dispatch={GetThreadDesktop(GetCurrentThreadId()),quit,stop,work,ready,0,next,&s};
    HANDLE thread=CreateThread(NULL,0,win32_process_global_dispatch_run,&dispatch,0,NULL);if(!thread)return 2;
    DWORD result=WaitForSingleObject(ready,3000);if(result!=WAIT_OBJECT_0 || dispatch.error)return 3;
    SetEvent(work);if(WaitForSingleObject(done,3000)!=WAIT_OBJECT_0)return 4;
    SetEvent(quit);if(WaitForSingleObject(thread,3000)!=WAIT_OBJECT_0 || s.processed!=2)return 5;
    CloseHandle(thread);CloseHandle(ready);
    ResetEvent(quit);ResetEvent(done);
    win32_process_global_dispatch_pool pool={0};
    blocked_state block={CreateEventW(NULL,TRUE,FALSE,NULL),CreateEventW(NULL,TRUE,FALSE,NULL)};
    if(!block.entered || !block.release)return 6;
    if(win32_process_global_dispatch_pool_start(&pool,2,GetThreadDesktop(GetCurrentThreadId()),quit,stop,work,blocked,&block,3000))return 7;
    SetEvent(work);if(WaitForSingleObject(block.entered,3000)!=WAIT_OBJECT_0)return 8;
    SetEvent(quit);
    if(win32_process_global_dispatch_pool_wait(&pool,GetTickCount64())!=ERROR_TIMEOUT)return 9;
    SetEvent(block.release);
    if(win32_process_global_dispatch_pool_wait(&pool,GetTickCount64()+3000))return 10;
    win32_process_global_dispatch_pool_release(&pool);
    ResetEvent(quit);
    if(win32_process_global_dispatch_pool_start(&pool,1,GetThreadDesktop(GetCurrentThreadId()),quit,stop,work,next,&s,3000))return 11;
    SetEvent(quit);if(win32_process_global_dispatch_pool_wait(&pool,GetTickCount64()+3000))return 12;
    win32_process_global_dispatch_pool_release(&pool);
    CloseHandle(block.entered);CloseHandle(block.release);CloseHandle(done);CloseHandle(work);CloseHandle(stop);CloseHandle(quit);
    puts("PROCESS_GLOBAL_DISPATCH_PASS ready=1 drain=2 quit=1 pool_timeout_retry=1 restart=1");return 0;
}
