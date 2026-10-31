#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_discover.h"

typedef struct owned_target {PROCESS_INFORMATION process;HANDLE ready,stop,late;} owned_target;
typedef struct observations {ntw16s_global_discover *watcher;DWORD first,second;unsigned existing,created,late,invalid;DWORD nested_stop;} observations;
static void discovered(HWND window,DWORD process,DWORD thread,void *parameter)
{
    observations *s=(observations *)parameter;DWORD actual=0;
    if(GetWindowThreadProcessId(window,&actual)!=thread || actual!=process)++s->invalid;
    if(process==s->first){WCHAR text[32];++s->existing;if(GetWindowTextW(window,text,ARRAYSIZE(text)) && !lstrcmpW(text,L"late"))++s->late;}
    if(process==s->second)++s->created;
    if(!s->nested_stop)s->nested_stop=ntw16s_global_discover_stop(s->watcher);
}
static DWORD WINAPI wrong_thread(void *parameter)
{return ntw16s_global_discover_stop((ntw16s_global_discover *)parameter);}
static int target(void)
{
    WCHAR ready_name[128],stop_name[128],late_name[128];
    if(!GetEnvironmentVariableW(L"WIN16_DISCOVER_READY",ready_name,ARRAYSIZE(ready_name)) ||
       !GetEnvironmentVariableW(L"WIN16_DISCOVER_STOP",stop_name,ARRAYSIZE(stop_name)) ||
       !GetEnvironmentVariableW(L"WIN16_DISCOVER_LATE",late_name,ARRAYSIZE(late_name)))return 2;
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name),stop=OpenEventW(SYNCHRONIZE,FALSE,stop_name),late=OpenEventW(SYNCHRONIZE|EVENT_MODIFY_STATE,FALSE,late_name);
    if(!ready || !stop || !late)return 3;
    HWND window=CreateWindowW(L"STATIC",L"owned-discovery",WS_OVERLAPPEDWINDOW,0,0,150,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window || !CreateWindowW(L"BUTTON",L"child",WS_CHILD|WS_VISIBLE,4,4,60,24,window,NULL,GetModuleHandleW(NULL),NULL))return 4;
    ShowWindow(window,SW_SHOW);SetEvent(ready);CloseHandle(ready);
    ULONGLONG deadline=GetTickCount64()+8000;
    while(GetTickCount64()<deadline){
        HANDLE waits[]={stop,late};DWORD result=MsgWaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,250,QS_ALLINPUT);
        if(result==WAIT_OBJECT_0)break;
        if(result==WAIT_OBJECT_0+1){ResetEvent(late);CreateWindowW(L"BUTTON",L"late",WS_CHILD|WS_VISIBLE,4,32,60,24,window,NULL,GetModuleHandleW(NULL),NULL);}
        if(result==WAIT_OBJECT_0+ARRAYSIZE(waits)){MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}}
    }
    DestroyWindow(window);CloseHandle(stop);CloseHandle(late);return 0;
}
static BOOL launch(owned_target *t,unsigned index)
{
    WCHAR ready[128],stop[128],late[128],self[MAX_PATH],cmd[MAX_PATH+32];
    swprintf(ready,ARRAYSIZE(ready),L"Local\\Win16DiscoverReady-%lu-%u",GetCurrentProcessId(),index);
    swprintf(stop,ARRAYSIZE(stop),L"Local\\Win16DiscoverStop-%lu-%u",GetCurrentProcessId(),index);
    swprintf(late,ARRAYSIZE(late),L"Local\\Win16DiscoverLate-%lu-%u",GetCurrentProcessId(),index);
    SetEnvironmentVariableW(L"WIN16_DISCOVER_READY",ready);SetEnvironmentVariableW(L"WIN16_DISCOVER_STOP",stop);
    SetEnvironmentVariableW(L"WIN16_DISCOVER_LATE",late);
    t->ready=CreateEventW(NULL,TRUE,FALSE,ready);t->stop=CreateEventW(NULL,TRUE,FALSE,stop);t->late=CreateEventW(NULL,TRUE,FALSE,late);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};
    return t->ready && t->stop && t->late && CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&t->process) &&
           WaitForSingleObject(t->ready,3000)==WAIT_OBJECT_0;
}
static void cleanup(owned_target *t)
{
    if(t->stop)SetEvent(t->stop);
    if(t->process.hProcess){if(WaitForSingleObject(t->process.hProcess,3000)!=WAIT_OBJECT_0)TerminateProcess(t->process.hProcess,ERROR_TIMEOUT);
        CloseHandle(t->process.hThread);CloseHandle(t->process.hProcess);}
    if(t->ready)CloseHandle(t->ready);
    if(t->stop)CloseHandle(t->stop);
    if(t->late)CloseHandle(t->late);
}
static void pump(unsigned milliseconds)
{
    ULONGLONG deadline=GetTickCount64()+milliseconds;
    while(GetTickCount64()<deadline){DWORD result=MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
        if(result==WAIT_OBJECT_0){MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}}}
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target();
    WCHAR desktop[128];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 5;
    owned_target targets[3]={0};ntw16s_global_discover watcher={0},duplicate={0};observations s={.watcher=&watcher};BOOL pass=FALSE;
    if(!launch(&targets[0],1))goto done;
    s.first=targets[0].process.dwProcessId;
    if(ntw16s_global_discover_start(&watcher,discovered,&s) || !s.existing || s.nested_stop!=ERROR_BUSY)goto done;
    SetEvent(targets[0].late);pump(150);if(!s.late)goto done;
    if(ntw16s_global_discover_start(&duplicate,discovered,&s)!=ERROR_BUSY)goto done;
    HANDLE other=CreateThread(NULL,0,wrong_thread,&watcher,0,NULL);DWORD other_code=0;
    if(!other || WaitForSingleObject(other,3000)!=WAIT_OBJECT_0 || !GetExitCodeThread(other,&other_code) || other_code!=ERROR_INVALID_PARAMETER)goto done;
    CloseHandle(other);
    if(!launch(&targets[1],2))goto done;
    s.second=targets[1].process.dwProcessId;
    pump(250);if(!s.created || s.invalid)goto done;
    if(ntw16s_global_discover_stop(&watcher))goto done;
    unsigned prior=s.created+s.existing;if(!launch(&targets[2],3))goto done;pump(150);
    if(s.created+s.existing!=prior)goto done;
    pass=TRUE;
done:
    if(watcher.hook)ntw16s_global_discover_stop(&watcher);
    for(unsigned i=0;i<ARRAYSIZE(targets);++i)cleanup(&targets[i]);
    if(!pass){printf("DISCOVER FAIL existing=%u created=%u late=%u invalid=%u nested=%lu\n",s.existing,s.created,s.late,s.invalid,s.nested_stop);return 6;}
    printf("DISCOVER PASS bits=%u existing=1 new=1 late=1 stop=1 nested=1 wrong_thread=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
