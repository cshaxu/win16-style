#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
int wmain(int argc,WCHAR **argv)
{
    if(argc<3)return 2;
    BOOL direct=!lstrcmpW(argv[1],L"--direct");
    WCHAR command[8192];int used=direct?swprintf(command,8192,L"\"%ls\"",argv[2]):swprintf(command,8192,L"\"%ls\" --wait \"%ls\"",argv[1],argv[2]);
    if(used<0)return 3;
    for(int i=3;i<argc;++i){int n=swprintf(command+used,8192-(size_t)used,L" \"%ls\"",argv[i]);if(n<0)return 3;used+=n;}
    HANDLE job=CreateJobObjectW(NULL,NULL);if(!job)return 4;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if(!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))){CloseHandle(job);return 5;}
    STARTUPINFOW startup={0};startup.cb=sizeof(startup);PROCESS_INFORMATION process={0};
    if(!CreateProcessW(direct?argv[2]:argv[1],command,NULL,NULL,TRUE,CREATE_SUSPENDED,NULL,NULL,&startup,&process)){CloseHandle(job);return 6;}
    if(!AssignProcessToJobObject(job,process.hProcess)){TerminateProcess(process.hProcess,7);CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);return 7;}
    ResumeThread(process.hThread);CloseHandle(process.hThread);
    DWORD wait=WaitForSingleObject(process.hProcess,35000),code=99;
    if(wait==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&code);
    else printf("DIALOG_DRIVER_TIMEOUT wait=%lu\n",wait);
    CloseHandle(job);CloseHandle(process.hProcess);
    printf("DIALOG_DRIVER exit=%lu\n",code);return (int)code;
}
