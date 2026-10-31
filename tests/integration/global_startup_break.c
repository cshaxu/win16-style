#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"

int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 2;
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],cmd[32768];DWORD bytes=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&bytes) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14) || !ntw16s_global_name(name,ARRAYSIZE(name)))return 3;
    BOOL allocated=FALSE;if(!GetConsoleCP()){if(!AllocConsole()){printf("BREAK console_error=%lu\n",GetLastError());return 4;}allocated=TRUE;}
    if(swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" --global",argv[1])<0)return 5;
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION manager={0};
    if(!CreateProcessW(argv[1],cmd,NULL,NULL,TRUE,CREATE_NEW_PROCESS_GROUP,NULL,NULL,&si,&manager))return 6;
    HANDLE map=NULL;ntw16s_global_session *s=NULL;DWORD phase=0,ready=0,host=0,code=99;BOOL sent=FALSE,passed=FALSE;
    ULONGLONG deadline=GetTickCount64()+5000;
    while(GetTickCount64()<deadline){
        if(!map)map=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        if(map && !s)s=MapViewOfFile(map,FILE_MAP_READ,0,0,sizeof(*s));
        if(s && s->owner_pid==manager.dwProcessId && s->phase==NTW16S_GLOBAL_ACTIVE){
            phase=s->phase;ready=s->ready;host=s->host_pid;
            if(!ready){sent=GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT,manager.dwProcessId);break;}
        }
        if(WaitForSingleObject(manager.hProcess,0)!=WAIT_TIMEOUT)break;
        Sleep(1);
    }
    DWORD wait=WaitForSingleObject(manager.hProcess,8000);
    if(wait==WAIT_OBJECT_0)GetExitCodeProcess(manager.hProcess,&code);
    else{TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}
    DWORD final_phase=s?s->phase:0;BOOL owner_alive=FALSE;
    if(s && s->host_pid)host=s->host_pid;
    if(host){HANDLE h=OpenProcess(SYNCHRONIZE,FALSE,host);if(h){owner_alive=WaitForSingleObject(h,0)==WAIT_TIMEOUT;CloseHandle(h);}}
    passed=sent && phase==NTW16S_GLOBAL_ACTIVE && !ready && wait==WAIT_OBJECT_0 && !code && final_phase==NTW16S_GLOBAL_STOPPING && !owner_alive;
    printf("STARTUP BREAK sent=%d before_ready=%d exit=%lu phase=%lu host_alive=%d pass=%d\n",sent,!ready,code,final_phase,owner_alive,passed);
    if(s)UnmapViewOfFile(s);
    if(map)CloseHandle(map);
    CloseHandle(manager.hThread);CloseHandle(manager.hProcess);
    if(allocated)FreeConsole();
    return passed?0:1;
}
