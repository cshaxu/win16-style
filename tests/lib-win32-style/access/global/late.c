#include "lib-win32-style/access/global/late.h"
#include "lib-win32/process-global/identity.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"late line=%d error=%lu\n",__LINE__,GetLastError());result=1;goto done;}}while(0)
static struct {HANDLE entered,release;volatile LONG calls;UINT message;unsigned notifications;BOOL bad;} fixture;
static DWORD initialize(HMODULE module,DWORD cookie,void *context)
{
    (void)context;
    if(module!=GetModuleHandleW(NULL) || cookie!=901)fixture.bad=TRUE;
    LONG call=InterlockedIncrement(&fixture.calls);
    if(call==1)return ERROR_ACCESS_DENIED;
    SetEvent(fixture.entered);
    return WaitForSingleObject(fixture.release,2000)==WAIT_OBJECT_0?0:ERROR_TIMEOUT;
}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==fixture.message){
        if(a!=901 || b!=73)fixture.bad=TRUE;
        ++fixture.notifications;return 0;
    }
    return DefWindowProcW(window,message,a,b);
}
static BOOL idle(style_global_late *state)
{
    ULONGLONG deadline=GetTickCount64()+3000;
    do{
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE))DispatchMessageW(&message);
        if(!InterlockedCompareExchange(&state->background.running,0,0))return TRUE;
        MsgWaitForMultipleObjects(0,NULL,FALSE,10,QS_ALLINPUT);
    }while(GetTickCount64()<deadline);
    return FALSE;
}
int main(void)
{
    int result=0;WCHAR prefix[80],control[80],name[STYLE_GLOBAL_NAME_CAP];
    HANDLE mapping=NULL;style_global_session *session=NULL;HWND window=NULL;
    swprintf(prefix,ARRAYSIZE(prefix),L"Local\\StyleLateTest.%lu",GetCurrentProcessId());
    swprintf(control,ARRAYSIZE(control),L"StyleLateTest.Control.%lu",GetCurrentProcessId());
    style_global_namespace space={.prefix=prefix,.launcher=L"owned.exe",.module=L"owned.dll",.magic=47,.version=3};
    style_global_late_ops ops={&space,initialize,control,73,NULL};style_global_late state={.ops=&ops};
    CHECK(style_global_session_name(&space,name,ARRAYSIZE(name)));
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(*session),name);
    CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    session=MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session));CHECK(session);
    *session=(style_global_session){.magic=47,.version=3,.cookie=901,.phase=STYLE_GLOBAL_STOPPING,.owner_pid=GetCurrentProcessId()};
    CHECK(win32_process_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high));
    fixture.entered=CreateEventW(NULL,TRUE,FALSE,NULL);fixture.release=CreateEventW(NULL,TRUE,FALSE,NULL);
    CHECK(fixture.entered && fixture.release);
    fixture.message=RegisterWindowMessageW(control);CHECK(fixture.message);
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=window_proc,.lpszClassName=L"StyleLateOwnedTest"};
    CHECK(RegisterClassW(&cls));
    window=CreateWindowW(cls.lpszClassName,L"owned",WS_OVERLAPPEDWINDOW,0,0,120,80,NULL,NULL,cls.hInstance,NULL);CHECK(window);
    ShowWindow(window,SW_SHOWNOACTIVATE);
    CHECK(!style_global_late_schedule(&state,901) && idle(&state) && !fixture.calls);
    InterlockedExchange(&session->phase,STYLE_GLOBAL_ACTIVE);
    CHECK(!style_global_late_schedule(&state,901) && idle(&state));
    CHECK(fixture.calls==1 && !state.background.completed && !fixture.notifications);
    CHECK(!style_global_late_schedule(&state,901));
    CHECK(WaitForSingleObject(fixture.entered,2000)==WAIT_OBJECT_0);
    CHECK(!style_global_late_schedule(&state,901) && fixture.calls==2);
    SetEvent(fixture.release);CHECK(idle(&state));
    /* Completion is marked before notification enumeration finishes. Pump the
     * queued GUI notification after observing the worker return as well. */
    CHECK(idle(&state));
    CHECK(style_global_late_schedule(&state,901) && fixture.calls==2);
    CHECK(fixture.notifications==1 && !fixture.bad);
 done:
    if(fixture.release)SetEvent(fixture.release);
    if(!idle(&state))ExitProcess(2);
    if(window)DestroyWindow(window);
    if(fixture.entered)CloseHandle(fixture.entered);
    if(fixture.release)CloseHandle(fixture.release);
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(!result)puts("GLOBAL_LATE_OK no_style=1 inactive=1 failure_retry=1 busy=1 completed_once=1 module=1 notify=1 drained=1");
    return result;
}
