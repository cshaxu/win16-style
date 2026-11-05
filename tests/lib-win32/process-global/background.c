#include "lib-win32/process-global/background.h"
#include <stdio.h>
typedef struct test {win32_process_global_background state;HANDLE entered,release,destroyed;volatile LONG prepared,destroyed_count;BOOL fail;} test;
static void *prepare(DWORD key,void *context)
{test *s=context;(void)key;InterlockedIncrement(&s->prepared);return s->fail?NULL:s;}
static void run(DWORD key,void *task,void *context)
{test *s=task;(void)context;SetEvent(s->entered);WaitForSingleObject(s->release,3000);win32_process_global_background_mark(&s->state,key);}
static void destroy(void *task,void *context)
{test *s=task;(void)context;InterlockedIncrement(&s->destroyed_count);SetEvent(s->destroyed);}
int main(void)
{
    test s={0};s.entered=CreateEventW(NULL,TRUE,FALSE,NULL);s.release=CreateEventW(NULL,TRUE,FALSE,NULL);s.destroyed=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!s.entered || !s.release || !s.destroyed)return 1;
    const win32_process_global_background_ops ops={prepare,run,destroy};int error=0;
    if(win32_process_global_background_schedule(&s.state,42,&ops,&s) || WaitForSingleObject(s.entered,3000)!=WAIT_OBJECT_0)error=2;
    if(!error && (win32_process_global_background_schedule(&s.state,42,&ops,&s) || win32_process_global_background_schedule(&s.state,43,&ops,&s) || s.prepared!=1))error=3;
    win32_process_global_background_mark(&s.state,42);
    if(!error && !win32_process_global_background_schedule(&s.state,42,&ops,&s))error=4;
    SetEvent(s.release);
    if(WaitForSingleObject(s.destroyed,3000)!=WAIT_OBJECT_0)error=5;
    if(!error && (s.destroyed_count!=1 || !win32_process_global_background_schedule(&s.state,42,&ops,&s)))error=6;
    s.fail=TRUE;
    if(!error && (win32_process_global_background_schedule(&s.state,43,&ops,&s) || s.prepared!=2 || s.state.running))error=7;
    if(!error && win32_process_global_background_schedule(&s.state,0,&ops,&s))error=8;
    CloseHandle(s.destroyed);CloseHandle(s.release);CloseHandle(s.entered);
    if(error){printf("BACKGROUND FAIL %d\n",error);return error;}
    puts("BACKGROUND PASS busy=1 complete=1 failure=1 release=1");return 0;
}
