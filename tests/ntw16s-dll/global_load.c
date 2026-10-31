#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_load.h"

static HANDLE env_event(const WCHAR *key,DWORD access)
{WCHAR name[128];return GetEnvironmentVariableW(key,name,ARRAYSIZE(name))?OpenEventW(access,FALSE,name):NULL;}

int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target")){
        HANDLE ready=env_event(L"WIN16_LOAD_TEST_READY",EVENT_MODIFY_STATE),stop=env_event(L"WIN16_LOAD_TEST_STOP",SYNCHRONIZE);
        if(!ready || !stop)return 2;
        SetEvent(ready);DWORD result=WaitForSingleObject(stop,8000);CloseHandle(ready);CloseHandle(stop);
        return result==WAIT_OBJECT_0?0:3;
    }
    if(argc!=2)return 4;
    WCHAR desktop[128],names[4][128],self[MAX_PATH],cmd[MAX_PATH+32];DWORD needed=0;
    const WCHAR *vars[]={L"WIN16_LOAD_TEST_READY",L"WIN16_LOAD_TEST_STOP",L"WIN16_LOAD_TEST_ENTERED",L"WIN16_LOAD_TEST_GATE"};
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 5;
    HANDLE events[4]={0};
    for(unsigned i=0;i<ARRAYSIZE(events);++i){
        swprintf(names[i],ARRAYSIZE(names[i]),L"Local\\Win16LoadTest-%lu-%u",GetCurrentProcessId(),i);
        SetEnvironmentVariableW(vars[i],names[i]);events[i]=CreateEventW(NULL,TRUE,FALSE,names[i]);if(!events[i])return 6;
    }
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))return 7;
    BOOL started=WaitForSingleObject(events[0],3000)==WAIT_OBJECT_0;
    ntw16s_global_load ticket={0},invalid={0};void *worker=NULL;
    DWORD bad_pid=ntw16s_global_load_begin(pi.hProcess,pi.dwProcessId+1,argv[1],&invalid);
    DWORD bad_name=ntw16s_global_load_begin(pi.hProcess,pi.dwProcessId,L"C:\\wrong.dll",&invalid);
    DWORD begin=started?ntw16s_global_load_begin(pi.hProcess,pi.dwProcessId,argv[1],&ticket):ERROR_TIMEOUT;
    BOOL entered=!begin && WaitForSingleObject(events[2],3000)==WAIT_OBJECT_0;
    DWORD duplicate=entered?ntw16s_global_load_begin(pi.hProcess,pi.dwProcessId,argv[1],&ticket):ERROR_INVALID_STATE;
    DWORD timed=entered?ntw16s_global_load_finish(&ticket,0,&worker):ERROR_TIMEOUT;
    DWORD busy=entered?ntw16s_global_load_discard(&ticket):ERROR_INVALID_STATE;
    BOOL retained=ticket.process && ticket.thread && ticket.remote_path && ticket.path && !worker;
    SetEvent(events[3]);
    DWORD finished=begin?begin:ntw16s_global_load_finish(&ticket,5000,&worker);
    BOOL cleared=!ticket.process && !ticket.thread && !ticket.remote_path && !ticket.path && worker;
    SetEvent(events[1]);DWORD wait=WaitForSingleObject(pi.hProcess,3000),code=99;
    if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
    for(unsigned i=0;i<ARRAYSIZE(events);++i)ResetEvent(events[i]);
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);STARTUPINFOW dead_si={.cb=sizeof(dead_si)};PROCESS_INFORMATION dead={0};
    if(!CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&dead_si,&dead))return 9;
    BOOL dead_ready=WaitForSingleObject(events[0],3000)==WAIT_OBJECT_0;ntw16s_global_load dead_ticket={0};void *dead_worker=NULL;
    DWORD dead_begin=dead_ready?ntw16s_global_load_begin(dead.hProcess,dead.dwProcessId,argv[1],&dead_ticket):ERROR_TIMEOUT;
    BOOL dead_entered=!dead_begin && WaitForSingleObject(events[2],3000)==WAIT_OBJECT_0;
    BOOL killed=TerminateProcess(dead.hProcess,91);DWORD dead_wait=WaitForSingleObject(dead.hProcess,3000);
    DWORD dead_finish=dead_begin?dead_begin:ntw16s_global_load_finish(&dead_ticket,3000,&dead_worker);
    BOOL dead_cleared=!dead_ticket.process && !dead_ticket.thread && !dead_ticket.remote_path && !dead_ticket.path && !dead_worker;
    CloseHandle(dead.hThread);CloseHandle(dead.hProcess);for(unsigned i=0;i<ARRAYSIZE(events);++i)CloseHandle(events[i]);
    if(!started || bad_pid!=ERROR_INVALID_PARAMETER || bad_name!=ERROR_INVALID_NAME || begin || !entered || duplicate!=ERROR_BUSY || timed!=ERROR_TIMEOUT || busy!=ERROR_BUSY || !retained || finished || !cleared || wait!=WAIT_OBJECT_0 || code || !dead_ready || dead_begin || !dead_entered || !killed || dead_wait!=WAIT_OBJECT_0 || dead_finish!=ERROR_PROCESS_ABORTED || !dead_cleared){
        printf("GLOBAL LOAD FAIL bad_pid=%lu bad_name=%lu begin=%lu entered=%d duplicate=%lu timed=%lu busy=%lu retained=%d finished=%lu cleared=%d wait=%lu code=%lu dead_ready=%d dead_begin=%lu dead_entered=%d killed=%d dead_wait=%lu dead_finish=%lu dead_cleared=%d\n",bad_pid,bad_name,begin,entered,duplicate,timed,busy,retained,finished,cleared,wait,code,dead_ready,dead_begin,dead_entered,killed,dead_wait,dead_finish,dead_cleared);return 8;
    }
    printf("GLOBAL LOAD PASS bits=%u bad_pid=1 bad_name=1 duplicate=1 timeout=1 retained=1 completed=1 dead_target=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
