#include "background.h"
#include <stdlib.h>
typedef struct background_job {win32_process_global_background *state;win32_process_global_background_ops ops;void *context,*task;DWORD key;} background_job;
void win32_process_global_background_mark(win32_process_global_background *state,DWORD key)
{if(state && key)InterlockedExchange(&state->completed,(LONG)key);}
static DWORD WINAPI run_job(void *parameter)
{
    background_job *job=parameter;
    job->ops.run(job->key,job->task,job->context);
    InterlockedExchange(&job->state->running,0);
    job->ops.destroy(job->task,job->context);free(job);return 0;
}
BOOL win32_process_global_background_schedule(win32_process_global_background *state,DWORD key,const win32_process_global_background_ops *ops,void *context)
{
    if(!state || !key || !ops || !ops->prepare || !ops->run || !ops->destroy)return FALSE;
    if((DWORD)InterlockedCompareExchange(&state->completed,0,0)==key)return TRUE;
    if(InterlockedCompareExchange(&state->running,1,0)!=0)return FALSE;
    void *task=ops->prepare(key,context);if(!task){InterlockedExchange(&state->running,0);return FALSE;}
    background_job *job=malloc(sizeof(*job));
    if(!job){ops->destroy(task,context);InterlockedExchange(&state->running,0);return FALSE;}
    *job=(background_job){state,*ops,context,task,key};
    HANDLE thread=CreateThread(NULL,0,run_job,job,0,NULL);
    if(!thread){ops->destroy(task,context);free(job);InterlockedExchange(&state->running,0);return FALSE;}
    CloseHandle(thread);return FALSE;
}
