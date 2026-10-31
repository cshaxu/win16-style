#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
#include "ntw16s-dll/global_thread.h"

LRESULT CALLBACK Ntw16sGlobalHook(int code,WPARAM a,LPARAM b)
{
    if(code>=0 && b && ((CWPSTRUCT *)b)->message==WM_APP+51){
        WCHAR name[128];
        if(GetEnvironmentVariableW(L"WIN16_THREAD_TEST_HIT",name,ARRAYSIZE(name))){
            HANDLE event=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);
            if(event){SetEvent(event);CloseHandle(event);}
        }
    }
    return CallNextHookEx(NULL,code,a,b);
}

static int target(void)
{
    ntw16s_global_thread_packet packet={sizeof(packet),1,GetCurrentThreadId(),0x12345};
    ntw16s_global_thread_packet invalid=packet;invalid.cookie++;
    HANDLE bad=CreateThread(NULL,0,ntw16s_global_thread_worker,&invalid,0,NULL);
    DWORD code=0;
    if(!bad || WaitForSingleObject(bad,3000)!=WAIT_OBJECT_0 || !GetExitCodeThread(bad,&code) || code!=ERROR_ACCESS_DENIED)return 2;
    CloseHandle(bad);
    HWND w=CreateWindowW(L"STATIC",L"worker-test",WS_OVERLAPPEDWINDOW,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!w)return 3;
    HANDLE worker=CreateThread(NULL,0,ntw16s_global_thread_worker,&packet,0,NULL);
    if(!worker)return 4;
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],event_name[NTW16S_GLOBAL_NAME_CAP+64];
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(event_name,ARRAYSIZE(event_name),L"%ls.thread.%lu.%lu.%lu",name,
       GetCurrentProcessId(),packet.thread_id,packet.cookie)<0)return 5;
    HANDLE ready=OpenEventW(SYNCHRONIZE,FALSE,event_name);
    if(!ready || WaitForSingleObject(ready,3000)!=WAIT_OBJECT_0)return 6;
    CloseHandle(ready);
    SendMessageW(w,WM_APP+51,0,0);
    if(WaitForSingleObject(worker,3000)!=WAIT_OBJECT_0 || !GetExitCodeThread(worker,&code))return 7;
    WCHAR hit_name[128];
    HANDLE hit=GetEnvironmentVariableW(L"WIN16_THREAD_TEST_HIT",hit_name,ARRAYSIZE(hit_name))?
        OpenEventW(EVENT_MODIFY_STATE|SYNCHRONIZE,FALSE,hit_name):NULL;
    if(!hit || !ResetEvent(hit))return 9;
    SendMessageW(w,WM_APP+51,0,0);
    if(WaitForSingleObject(hit,100)!=WAIT_TIMEOUT)return 10;
    CloseHandle(hit);CloseHandle(worker);DestroyWindow(w);
    return code?8:0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target();
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],
          ready_name[NTW16S_GLOBAL_NAME_CAP+64],hit_name[128],self[MAX_PATH],cmd[MAX_PATH+32];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 10;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0)return 11;
    HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!map || GetLastError()==ERROR_ALREADY_EXISTS)return 12;
    ntw16s_global_session *s=(ntw16s_global_session *)MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(*s));if(!s)return 13;
    ZeroMemory(s,sizeof(*s));s->magic=NTW16S_GLOBAL_MAGIC;s->version=NTW16S_GLOBAL_VERSION;
    s->owner_pid=GetCurrentProcessId();s->cookie=0x12345;s->phase=NTW16S_GLOBAL_ACTIVE;
    if(!ntw16s_global_birth(GetCurrentProcess(),&s->owner_birth_low,&s->owner_birth_high))return 14;
    swprintf(hit_name,ARRAYSIZE(hit_name),L"Local\\Win16ThreadTest-%lu",GetCurrentProcessId());
    SetEnvironmentVariableW(L"WIN16_THREAD_TEST_HIT",hit_name);
    HANDLE stop=CreateEventW(NULL,TRUE,FALSE,stop_name),hit=CreateEventW(NULL,TRUE,FALSE,hit_name);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!stop || !hit || !CreateProcessW(self,cmd,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&si,&pi))return 15;
    swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.thread.%lu.%lu.%lu",name,pi.dwProcessId,pi.dwThreadId,s->cookie);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name);if(!ready)return 16;
    ResumeThread(pi.hThread);
    BOOL observed=WaitForSingleObject(hit,5000)==WAIT_OBJECT_0;SetEvent(stop);
    DWORD wait=WaitForSingleObject(pi.hProcess,5000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
    else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(ready);CloseHandle(hit);CloseHandle(stop);
    UnmapViewOfFile(s);CloseHandle(map);
    if(!observed || wait!=WAIT_OBJECT_0 || code)return 17;
    printf("GLOBAL THREAD WORKER PASS bits=%u callback=1 stop=1 bad_cookie=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
