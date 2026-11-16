#include "lib-win32-style/access/global/control.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"control line=%d error=%lu\n",__LINE__,GetLastError());result=1;goto done;}}while(0)
typedef struct fixture {
    HANDLE ready,stop,release;style_global_session *session;
    volatile LONG running,overlap;DWORD code,last_error;unsigned failures;
    BOOL hold;
} fixture;
static DWORD run(HANDLE parent,DWORD style,void *context)
{
    fixture *f=context;(void)parent;
    if(InterlockedIncrement(&f->running)!=1)InterlockedIncrement(&f->overlap);
    f->session->style=style;InterlockedExchange(&f->session->phase,STYLE_GLOBAL_ACTIVE);
    InterlockedExchange(&f->session->ready,TRUE);SetEvent(f->ready);
    if(f->hold)WaitForSingleObject(f->release,5000);
    DWORD code=WaitForSingleObject(f->stop,5000)==WAIT_OBJECT_0?f->code:ERROR_TIMEOUT;
    InterlockedExchange(&f->session->ready,FALSE);InterlockedExchange(&f->session->phase,STYLE_GLOBAL_STOPPING);
    InterlockedDecrement(&f->running);return code;
}
static void failed(DWORD error,void *context)
{fixture *f=context;++f->failures;f->last_error=error;}
static BOOL ready(style_global_control *state,fixture *f,DWORD style)
{
    if(WaitForSingleObject(f->ready,2000)!=WAIT_OBJECT_0)return FALSE;
    style_global_control_update(state);
    return state->enabled && !state->busy && state->style==style;
}
static BOOL finish(style_global_control *state,fixture *f)
{
    if(WaitForSingleObject(state->worker,2000)!=WAIT_OBJECT_0)return FALSE;
    ResetEvent(f->stop);ResetEvent(f->ready);DWORD code=0;
    return !style_global_control_finished(state,&code);
}
int main(void)
{
    int result=0;fixture f={0};style_global_control state={0};HANDLE mapping=NULL;
    WCHAR prefix[80];swprintf(prefix,ARRAYSIZE(prefix),L"Local\\StyleControlTest.%lu",GetCurrentProcessId());
    style_global_namespace space={.prefix=prefix,.launcher=L"owned.exe",.module=L"owned.dll",.magic=7,.version=9};
    const style_global_control_ops ops={run,failed,77};
    CHECK(style_global_control_init(&state,NULL,41,500,&space,&ops,&f));
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(*f.session),state.session_name);
    CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    f.session=MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*f.session));CHECK(f.session);
    *f.session=(style_global_session){.magic=space.magic,.version=space.version,.owner_pid=GetCurrentProcessId()};
    f.ready=CreateEventW(NULL,TRUE,FALSE,NULL);f.stop=CreateEventW(NULL,TRUE,FALSE,state.request_name);
    f.release=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(f.ready && f.stop && f.release);
    style_global_control_select(&state,41);CHECK(ready(&state,&f,41));
    style_global_control_select(&state,41);CHECK(finish(&state,&f) && !state.worker && !state.enabled);
    style_global_control_select(&state,99);CHECK(ready(&state,&f,99));
    style_global_control_select(&state,99);CHECK(finish(&state,&f) && !state.worker);
    style_global_control_select(&state,41);CHECK(ready(&state,&f,41));
    style_global_control_select(&state,99);CHECK(finish(&state,&f) && ready(&state,&f,99));
    style_global_control_select(&state,41);CHECK(finish(&state,&f) && ready(&state,&f,41));
    style_global_control_select(&state,99);style_global_control_select(&state,41);
    CHECK(finish(&state,&f) && ready(&state,&f,41));
    style_global_control_select(&state,41);CHECK(finish(&state,&f) && !state.worker);
    f.code=ERROR_ACCESS_DENIED;style_global_control_select(&state,99);CHECK(ready(&state,&f,99));
    style_global_control_select(&state,99);CHECK(finish(&state,&f));
    CHECK(f.failures==1 && f.last_error==ERROR_ACCESS_DENIED && state.desired==500 && !state.busy);
    f.code=0;f.hold=TRUE;style_global_control_select(&state,41);CHECK(ready(&state,&f,41));
    CHECK(style_global_control_close(&state,0)==ERROR_TIMEOUT && state.worker);
    SetEvent(f.release);CHECK(!style_global_control_close(&state,2000) && !state.worker);
    CHECK(!f.running && !f.overlap);
 done:
    if(f.release)SetEvent(f.release);
    if(f.stop)SetEvent(f.stop);
    if(state.worker && style_global_control_close(&state,6000))ExitProcess(2);
    if(f.ready)CloseHandle(f.ready);
    if(f.stop)CloseHandle(f.stop);
    if(f.release)CloseHandle(f.release);
    if(f.session)UnmapViewOfFile(f.session);
    if(mapping)CloseHandle(mapping);
    if(!result)puts("GLOBAL_CONTROL_OK six_edges=1 opaque_ids=41_99_500 ready=1 rapid_choice=1 no_overlap=1 failure=1 retained_timeout=1 joined=1");
    return result;
}
