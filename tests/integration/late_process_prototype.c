#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "ntw16s-dll/injection/intercept.h"
#include "ntw16s-dll/injection/late_process.h"
#include "ntw16s-dll/global_session.h"
ntw16s_context ntw16s_process_context;
static volatile LONG running=1,ticks;
/* The ordinary-startup backend is not linked into this isolated fixture. */
DWORD ntw16s_hooks_attach(void *slot,void *replacement)
{(void)slot;(void)replacement;return ERROR_NOT_SUPPORTED;}
static DWORD WINAPI background(void *p)
{(void)p;while(InterlockedCompareExchange(&running,0,0)){InterlockedIncrement(&ticks);Sleep(1);}return 0;}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"existing")){
        BYTE before_w[16],before_a[16];memcpy(before_w,(const void *)CreateProcessW,sizeof(before_w));memcpy(before_a,(const void *)CreateProcessA,sizeof(before_a));
        ntw16s_process_context.flags=NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE;
        DWORD status=ntw16s_late_process_init(L"C:\\fixture",123);
        BOOL unchanged=!memcmp(before_w,(const void *)CreateProcessW,sizeof(before_w)) && !memcmp(before_a,(const void *)CreateProcessA,sizeof(before_a));
        printf("LATE EXISTING bits=%u status=%lu unchanged=%d\n",(unsigned)(sizeof(void*)*8),status,unchanged);
        return !status && unchanged?0:1;
    }
    if(argc!=4 || (wcscmp(argv[3],L"A") && wcscmp(argv[3],L"W")))return 2;
    WCHAR directory[NTW16S_PATH_CAP];DWORD length=GetFullPathNameW(argv[2],NTW16S_PATH_CAP,directory,NULL);
    if(!length || length>=NTW16S_PATH_CAP)return 3;
    WCHAR session_name[NTW16S_GLOBAL_NAME_CAP];if(!ntw16s_global_name(session_name,ARRAYSIZE(session_name)))return 9;
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),session_name);
    if(!mapping || GetLastError()==ERROR_ALREADY_EXISTS)return 9;
    ntw16s_global_session *session=MapViewOfFile(mapping,FILE_MAP_WRITE,0,0,sizeof(*session));if(!session)return 9;
    session->magic=NTW16S_GLOBAL_MAGIC;session->version=NTW16S_GLOBAL_VERSION;session->owner_pid=GetCurrentProcessId();session->cookie=0x513a9187;
    if(!ntw16s_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high))return 9;
    session->phase=NTW16S_GLOBAL_ACTIVE;
    SetLastError(777);BOOL active=ntw16s_global_policy_active(session->cookie),wrong=ntw16s_global_policy_active(session->cookie^1u);
    if(!active || wrong || GetLastError()!=777)return 10;
    session->phase=NTW16S_GLOBAL_STOPPING;if(ntw16s_global_policy_active(session->cookie))return 10;
    session->phase=NTW16S_GLOBAL_ACTIVE;
    HANDLE thread=CreateThread(NULL,0,background,NULL,0,NULL);if(!thread)return 4;
    while(!InterlockedCompareExchange(&ticks,0,0))Sleep(1);
    DWORD error=ntw16s_late_process_init(directory,session->cookie);
    if(error){printf("LATE PROCESS install=%lu\n",error);return 6;}
    DWORD repeat=ntw16s_late_process_init(directory,session->cookie);if(repeat)return 5;
    PROCESS_INFORMATION child={0};BOOL made=FALSE;
    if(!wcscmp(argv[3],L"W")){
        WCHAR command[32768];STARTUPINFOW si={.cb=sizeof(si)};
        if(swprintf(command,ARRAYSIZE(command),L"\"%ls\"",argv[1])<0)return 7;
        made=CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child);
    }else{
        char image[32768],command[32768];STARTUPINFOA si={.cb=sizeof(si)};BOOL replaced=FALSE,utf8=GetACP()==CP_UTF8;
        if(!WideCharToMultiByte(CP_ACP,utf8?0:WC_NO_BEST_FIT_CHARS,argv[1],-1,image,sizeof(image),NULL,utf8?NULL:&replaced)||replaced)return 8;
        if(snprintf(command,sizeof(command),"\"%s\"",image)<0)return 8;
        made=CreateProcessA(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child);
    }
    DWORD code=99;if(made){if(WaitForSingleObject(child.hProcess,10000)==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);else{TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}CloseHandle(child.hThread);CloseHandle(child.hProcess);}
    WCHAR ready_name[128],go_name[128],wait_command[32768];
    swprintf(ready_name,ARRAYSIZE(ready_name),L"Local\\Win16ProbeReady.%lu",GetCurrentProcessId());
    swprintf(go_name,ARRAYSIZE(go_name),L"Local\\Win16ProbeGo.%lu",GetCurrentProcessId());
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name),go=CreateEventW(NULL,TRUE,FALSE,go_name);
    PROCESS_INFORMATION waiting={0};STARTUPINFOW wait_si={.cb=sizeof(wait_si)};BOOL waiting_created=FALSE;
    if(ready && go && swprintf(wait_command,ARRAYSIZE(wait_command),L"\"%ls\" wait %ls %ls",argv[1],ready_name,go_name)>=0)
        waiting_created=CreateProcessW(argv[1],wait_command,NULL,NULL,TRUE,0,NULL,NULL,&wait_si,&waiting);
    BOOL waiting_ready=waiting_created && WaitForSingleObject(ready,5000)==WAIT_OBJECT_0;
    session->phase=NTW16S_GLOBAL_STOPPING;if(go)SetEvent(go);
    DWORD waiting_code=99;if(waiting_created){if(WaitForSingleObject(waiting.hProcess,7000)==WAIT_OBJECT_0)GetExitCodeProcess(waiting.hProcess,&waiting_code);else{TerminateProcess(waiting.hProcess,ERROR_TIMEOUT);WaitForSingleObject(waiting.hProcess,1000);}CloseHandle(waiting.hThread);CloseHandle(waiting.hProcess);}
    if(ready)CloseHandle(ready);
    if(go)CloseHandle(go);
    PROCESS_INFORMATION after={0};BOOL stopped_created=FALSE;
    if(!wcscmp(argv[3],L"W")){
        WCHAR command[32768];STARTUPINFOW si={.cb=sizeof(si)};
        if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" child",argv[1])>=0)stopped_created=CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&si,&after);
    }else{
        char image[32768],command[32768];STARTUPINFOA si={.cb=sizeof(si)};BOOL replaced=FALSE,utf8=GetACP()==CP_UTF8;
        if(WideCharToMultiByte(CP_ACP,utf8?0:WC_NO_BEST_FIT_CHARS,argv[1],-1,image,sizeof(image),NULL,utf8?NULL:&replaced) && !replaced && snprintf(command,sizeof(command),"\"%s\" child",image)>0)
            stopped_created=CreateProcessA(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&after);
    }
    DWORD stopped_code=99;if(stopped_created){if(WaitForSingleObject(after.hProcess,5000)==WAIT_OBJECT_0)GetExitCodeProcess(after.hProcess,&stopped_code);else{TerminateProcess(after.hProcess,ERROR_TIMEOUT);WaitForSingleObject(after.hProcess,1000);}CloseHandle(after.hThread);CloseHandle(after.hProcess);}
    InterlockedExchange(&running,0);DWORD joined=WaitForSingleObject(thread,2000);CloseHandle(thread);
    printf("LATE PROCESS bits=%u api=%ls init=1 repeat=1 created=%d exit=%lu stopped=%d stopped_exit=%lu descendant=%d joined=%d\n",(unsigned)(sizeof(void*)*8),argv[3],made,code,stopped_created,stopped_code,waiting_ready&&waiting_code==0,joined==WAIT_OBJECT_0);
    UnmapViewOfFile(session);CloseHandle(mapping);
    /* Test process exits with detours and trampoline storage still live. */
    return made && code==0 && stopped_created && stopped_code==41 && waiting_ready && waiting_code==0 && joined==WAIT_OBJECT_0?0:1;
}
