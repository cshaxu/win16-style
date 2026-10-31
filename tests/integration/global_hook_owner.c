#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
int wmain(int argc,WCHAR **argv)
{
    if(argc!=3)return 2;
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],command[32768];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed)||wcsncmp(desktop,L"Win16S-Dialog-",14))return 3;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)))return 4;
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!mapping || GetLastError()==ERROR_ALREADY_EXISTS)return 5;
    ntw16s_global_session *s=MapViewOfFile(mapping,FILE_MAP_WRITE,0,0,sizeof(*s));if(!s)return 6;
    s->magic=NTW16S_GLOBAL_MAGIC;s->version=NTW16S_GLOBAL_VERSION;s->owner_pid=GetCurrentProcessId();s->cookie=0x672ad713;
    if(!ntw16s_global_birth(GetCurrentProcess(),&s->owner_birth_low,&s->owner_birth_high))return 7;
    s->phase=NTW16S_GLOBAL_ACTIVE;STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION child={0};
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" localhook \"%ls\"",argv[1],argv[2])<0 || !CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child))return 8;
    DWORD code=99;if(WaitForSingleObject(child.hProcess,12000)==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);
    else{TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}
    CloseHandle(child.hThread);CloseHandle(child.hProcess);s->phase=NTW16S_GLOBAL_STOPPING;UnmapViewOfFile(s);CloseHandle(mapping);
    printf("LOCAL GLOBAL HOOK target_exit=%lu\n",code);return code?1:0;
}
