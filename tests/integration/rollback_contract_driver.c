#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 300000
#include "../ntw16s-dll/isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ROLLBACK MATRIX FAIL %d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static int check_report(PCWSTR path,unsigned windows)
{
    FILE *file=NULL;char line[200];unsigned complete=0,attached=0;if(_wfopen_s(&file,path,L"rt"))return 0;
    while(fgets(line,sizeof(line),file)){if(strstr(line,"FAIL")){fclose(file);return 0;}
        complete+=!strncmp(line,"complete ",9);attached+=!strncmp(line,"window-attached ",16);}
    fclose(file);return complete==1 && attached==windows;
}
static int matrix(int argc,WCHAR **argv)
{
    WCHAR scratch[MAX_PATH],folder[MAX_PATH],launcher[MAX_PATH],paths[4][MAX_PATH],command[32768];
    CHECK(argc==6 && GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    static const WCHAR *const names[]={L"ntw16s32.dll",L"ntw16s64.dll",L"rollback32.exe",L"rollback64.exe"};
    for(unsigned root=0;root<2;++root)for(unsigned ansi=0;ansi<2;++ansi){
        CHECK(swprintf_s(folder,MAX_PATH,L"%ls\\rollback-r%ua%u",scratch,root,ansi)>0 && CreateDirectoryW(folder,NULL));
        CHECK(swprintf_s(launcher,MAX_PATH,L"%ls\\run16s.exe",folder)>0 && CopyFileW(argv[1],launcher,FALSE));
        for(unsigned i=0;i<4;++i)CHECK(swprintf_s(paths[i],MAX_PATH,L"%ls\\%ls",folder,names[i])>0 && CopyFileW(argv[2+i],paths[i],FALSE));
        CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\" --parent \"%ls\" \"%ls\" \"%ls\" \"%ls\" %u %u",launcher,paths[2+root],paths[3-root],paths[2+root],folder,folder,ansi,root?64:32)>0);
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        STARTUPINFOW startup={sizeof(startup)};PROCESS_INFORMATION process={0};DWORD code;
        CHECK(CreateProcessW(launcher,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&startup,&process));
        if(!AssignProcessToJobObject(job,process.hProcess)){TerminateProcess(process.hProcess,1);WaitForSingleObject(process.hProcess,1000);CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);CHECK(FALSE);}
        CHECK(ResumeThread(process.hThread)==1);DWORD wait=WaitForSingleObject(process.hProcess,60000);
        if(wait==WAIT_OBJECT_0)CHECK(GetExitCodeProcess(process.hProcess,&code));else code=ERROR_TIMEOUT;
        CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
        WCHAR report[MAX_PATH];CHECK(swprintf_s(report,MAX_PATH,L"%ls\\parent.txt",folder)>0 && check_report(report,2));
        CHECK(swprintf_s(report,MAX_PATH,L"%ls\\sibling.txt",folder)>0 && check_report(report,2));
        CHECK(swprintf_s(report,MAX_PATH,L"%ls\\next.txt",folder)>0 && check_report(report,2));
        CHECK(swprintf_s(report,MAX_PATH,L"%ls\\failed-entry.txt",folder)>0 && GetFileAttributesW(report)==INVALID_FILE_ATTRIBUTES);
        printf("ROLLBACK PASS parent=%u failed child=%u API=%s, exact job creation/exit, parent/sibling survived and restyled\n",root?64:32,root?32:64,ansi?"A":"W");
    }
    return 0;
}
int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,matrix);}
