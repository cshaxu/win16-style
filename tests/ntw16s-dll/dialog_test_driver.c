#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
static BOOL desktop_released(HDESK desktop,const WCHAR *name)
{
    if(!CloseDesktop(desktop)){printf("PRIVATE_DESKTOP_CLOSE_FAIL error=%lu\n",GetLastError());return FALSE;}
    for(unsigned i=0;i<10;++i){
        HDESK remaining=OpenDesktopW(name,0,FALSE,DESKTOP_READOBJECTS);
        if(!remaining){DWORD error=GetLastError();if(error==ERROR_FILE_NOT_FOUND || error==ERROR_INVALID_NAME)return TRUE;
            printf("PRIVATE_DESKTOP_CHECK_FAIL error=%lu\n",error);return FALSE;}
        CloseDesktop(remaining);Sleep(100);
    }
    wprintf(L"PRIVATE_DESKTOP_LEAK name=%ls\n",name);return FALSE;
}
static int desktop_self_test(void)
{
    WCHAR name[80];swprintf(name,ARRAYSIZE(name),L"Win16S-LeakCheck-%lu-%lu",GetCurrentProcessId(),GetTickCount());
    HDESK first=CreateDesktopW(name,NULL,NULL,0,GENERIC_ALL,NULL);
    if(!first){printf("PRIVATE_DESKTOP_CREATE_FAIL error=%lu\n",GetLastError());return 2;}
    HDESK second=OpenDesktopW(name,0,FALSE,DESKTOP_READOBJECTS);
    if(!second){CloseDesktop(first);return 3;}
    BOOL incorrectly_released=desktop_released(first,name);CloseDesktop(second);
    HDESK remaining=OpenDesktopW(name,0,FALSE,DESKTOP_READOBJECTS);
    if(remaining){CloseDesktop(remaining);return 4;}
    DWORD error=GetLastError();if(error!=ERROR_FILE_NOT_FOUND && error!=ERROR_INVALID_NAME)return 6;
    if(incorrectly_released)return 5;
    printf("PRIVATE_DESKTOP_TEST PASS retained=1 released=1\n");return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"--self-test"))return desktop_self_test();
    if(argc<3)return 2;
    BOOL isolated=!lstrcmpW(argv[1],L"--private");int offset=isolated?1:0;
    if(argc<3+offset)return 2;
    BOOL direct=!lstrcmpW(argv[1+offset],L"--direct");
    WCHAR command[8192];int used=direct?swprintf(command,8192,L"\"%ls\"",argv[2+offset]):swprintf(command,8192,L"\"%ls\" --wait \"%ls\"",argv[1+offset],argv[2+offset]);
    if(used<0)return 3;
    for(int i=3+offset;i<argc;++i){int n=swprintf(command+used,8192-(size_t)used,L" \"%ls\"",argv[i]);if(n<0)return 3;used+=n;}
    HANDLE job=CreateJobObjectW(NULL,NULL);if(!job)return 4;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if(!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))){CloseHandle(job);return 5;}
    STARTUPINFOW startup={0};startup.cb=sizeof(startup);PROCESS_INFORMATION process={0};
    HDESK desktop=NULL;WCHAR desktop_name[80];
    if(isolated){swprintf(desktop_name,ARRAYSIZE(desktop_name),L"Win16S-Dialog-%lu-%lu",GetCurrentProcessId(),GetTickCount());
        desktop=CreateDesktopW(desktop_name,NULL,NULL,0,GENERIC_ALL,NULL);if(!desktop){printf("PRIVATE_DESKTOP_CREATE_FAIL error=%lu\n",GetLastError());CloseHandle(job);return 2;}startup.lpDesktop=desktop_name;}
    if(!CreateProcessW(direct?argv[2+offset]:argv[1+offset],command,NULL,NULL,TRUE,CREATE_SUSPENDED,NULL,NULL,&startup,&process)){if(desktop)CloseDesktop(desktop);CloseHandle(job);return 6;}
    if(!AssignProcessToJobObject(job,process.hProcess)){TerminateProcess(process.hProcess,7);CloseHandle(process.hThread);CloseHandle(process.hProcess);if(desktop)CloseDesktop(desktop);CloseHandle(job);return 7;}
    ResumeThread(process.hThread);CloseHandle(process.hThread);
    DWORD wait=WaitForSingleObject(process.hProcess,35000),code=99;
    if(wait==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&code);
    else printf("DIALOG_DRIVER_TIMEOUT wait=%lu\n",wait);
    CloseHandle(job);CloseHandle(process.hProcess);if(desktop && !desktop_released(desktop,desktop_name))code=8;
    printf("DIALOG_DRIVER exit=%lu\n",code);return (int)code;
}
