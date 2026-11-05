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
    CloseHandle(thread);CloseHandle(done);CloseHandle(ready);CloseHandle(work);CloseHandle(stop);CloseHandle(quit);
    puts("PROCESS_GLOBAL_DISPATCH_PASS ready=1 drain=2 quit=1");return 0;
}
