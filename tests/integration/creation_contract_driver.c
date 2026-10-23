#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 500000
#include "../ntw16s-dll/isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTRACT MATRIX FAIL %d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static int completed(PCWSTR path,const char *phase,unsigned expected)
{
    FILE *file=NULL;char line[200];unsigned count=0;
    if(_wfopen_s(&file,path,L"rt"))return 0;
    while(fgets(line,sizeof(line),file)){if(strstr(line,"FAIL")){fclose(file);return 0;}if(!strncmp(line,phase,strlen(phase)))++count;}
    fclose(file);return count==expected;
}
static int matrix(int argc,WCHAR **argv)
{
    WCHAR scratch[MAX_PATH],package[MAX_PATH],executables[MAX_PATH],launcher[MAX_PATH],files[4][MAX_PATH],unicode[2][MAX_PATH],command[32768];
    CHECK(argc==6 && GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    CHECK(swprintf_s(package,MAX_PATH,L"%ls\\contract package",scratch)>0 && CreateDirectoryW(package,NULL));
    CHECK(swprintf_s(executables,MAX_PATH,L"%ls\\separate executables",scratch)>0 && CreateDirectoryW(executables,NULL));
    CHECK(swprintf_s(launcher,MAX_PATH,L"%ls\\run16s.exe",package)>0 && CopyFileW(argv[1],launcher,FALSE));
    for(unsigned i=0;i<2;++i){
        CHECK(swprintf_s(files[i],MAX_PATH,L"%ls\\ntw16s%u.dll",package,i?64:32)>0 && CopyFileW(argv[2+i],files[i],FALSE));
        CHECK(swprintf_s(files[2+i],MAX_PATH,L"%ls\\contract%u.exe",executables,i?64:32)>0 && CopyFileW(argv[4+i],files[2+i],FALSE));
        CHECK(swprintf_s(unicode[i],MAX_PATH,L"%ls\\子程序 %u.exe",executables,i?64:32)>0 && CopyFileW(argv[4+i],unicode[i],FALSE));
    }
    for(unsigned root=0;root<2;++root)for(unsigned child=0;child<2;++child)for(unsigned styled=0;styled<2;++styled){
        WCHAR folder[MAX_PATH],tail[32768];STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION process={0};DWORD code;
        CHECK(swprintf_s(folder,MAX_PATH,L"%ls\\r%uc%us%u",scratch,root,child,styled)>0 && CreateDirectoryW(folder,NULL));
        CHECK(swprintf_s(tail,ARRAYSIZE(tail),L"\"%ls\" --parent \"%ls\" \"%ls\" %u %u \"%ls\" \"%ls\" %u",files[2+root],files[2+child],unicode[child],child?64:32,styled,package,folder,root?64:32)>0);
        if(styled)CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait %ls",launcher,tail)>0);
        else CHECK(!wcscpy_s(command,ARRAYSIZE(command),tail));
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        CHECK(CreateProcessW(styled?launcher:files[2+root],command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&startup,&process));
        if(!AssignProcessToJobObject(job,process.hProcess)){TerminateProcess(process.hProcess,1);WaitForSingleObject(process.hProcess,1000);CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);CHECK(FALSE);}
        CHECK(ResumeThread(process.hThread)==1);DWORD wait=WaitForSingleObject(process.hProcess,60000);
        if(wait==WAIT_OBJECT_0)CHECK(GetExitCodeProcess(process.hProcess,&code));else code=ERROR_TIMEOUT;
        CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);
        if(code)fprintf(stderr,"contract parent root=%u child=%u styled=%u exit=%lu\n",root?64:32,child?64:32,styled,code);
        CHECK(wait==WAIT_OBJECT_0 && !code);
        WCHAR path[MAX_PATH];CHECK(swprintf_s(path,MAX_PATH,L"%ls\\parent.txt",folder)>0 && completed(path,"complete ",1) && completed(path,"case-complete ",8));
        for(unsigned variant=0;variant<8;++variant)CHECK(swprintf_s(path,MAX_PATH,L"%ls\\v%u.txt",folder,variant)>0 && completed(path,"complete ",1) && completed(path,styled?"window-attached ":"window-native ",1));
        printf("CONTRACT PASS root=%u child=%u styled=%u vectors=8\n",root?64:32,child?64:32,styled);
    }
    puts("CREATION CONTRACT PASS 64 native/injected vectors, four edges, fields/environment/cwd/handles/EX/suspension");return 0;
}
int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,matrix);}
