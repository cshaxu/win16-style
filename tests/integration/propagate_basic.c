#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 300000
#include "../ntw16s-dll/isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TREE MATRIX FAIL %d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static int matrix(int argc,WCHAR **argv)
{
    WCHAR scratch[MAX_PATH],package[MAX_PATH],launcher[MAX_PATH],files[4][MAX_PATH],command[32768];
    CHECK(argc==6 && GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    CHECK(swprintf_s(package,MAX_PATH,L"%ls\\tree package",scratch)>0 && CreateDirectoryW(package,NULL));
    CHECK(swprintf_s(launcher,MAX_PATH,L"%ls\\run16s.exe",package)>0 && CopyFileW(argv[1],launcher,FALSE));
    static const WCHAR *const names[]={L"ntw16s32.dll",L"ntw16s64.dll",L"tree32.exe",L"tree64.exe"};
    for(unsigned i=0;i<4;++i)CHECK(swprintf_s(files[i],MAX_PATH,L"%ls\\%ls",package,names[i])>0 && CopyFileW(argv[2+i],files[i],FALSE));
    for(unsigned root=0;root<2;++root)for(unsigned child=0;child<2;++child)for(unsigned api=0;api<2;++api){
        WCHAR id[40];DWORD code;STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION process={0};
        CHECK(swprintf_s(id,40,L"r%uc%ua%u",root,child,api)>0);
        CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\" 2 \"%ls\" \"%ls\" %u %u \"%ls\"",launcher,files[2+root],files[2+child],files[3-root],api,(root+child+api)&1,id)>0);
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        CHECK(CreateProcessW(launcher,command,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&process));
        if(!AssignProcessToJobObject(job,process.hProcess)){TerminateProcess(process.hProcess,1);CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);CHECK(FALSE);}
        CHECK(ResumeThread(process.hThread)==1);
        DWORD wait=WaitForSingleObject(process.hProcess,60000);
        if(wait==WAIT_OBJECT_0)CHECK(GetExitCodeProcess(process.hProcess,&code));else code=ERROR_TIMEOUT;
        CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
        DWORD pids[3]={0},children[3]={0};unsigned widths[3]={1-root,child,root};
        for(unsigned rank=0;rank<3;++rank){
            WCHAR path[MAX_PATH];FILE *file=NULL;char line[160];unsigned complete=0,attached=0;
            CHECK(swprintf_s(path,MAX_PATH,L"%ls\\tree-%ls-g%u-b%u.txt",scratch,id,rank,widths[rank]?64:32)>0 && !_wfopen_s(&file,path,L"rt"));
            while(fgets(line,sizeof(line),file)){
                if(!strncmp(line,"entry ",6))CHECK(sscanf_s(line+6,"%lu",&pids[rank])==1);
                if(!strncmp(line,"child ",6))CHECK(sscanf_s(line+6,"%lu",&children[rank])==1);
                attached+=!strncmp(line,"window-attached ",16);complete+=!strncmp(line,"complete ",9);}
            fclose(file);CHECK(pids[rank] && attached==1 && complete==1);
        }
        CHECK(children[2]==pids[1] && children[1]==pids[0] && !children[0]);
        printf("TREE PASS root=%u child=%u grandchild=%u A/W=%u suspended=%u generations=3\n",root?64:32,child?64:32,root?32:64,api,(root+child+api)&1);
    }
    puts("PROPAGATION BASIC PASS four edges, both A/W, three generations, caller suspension, actual windows/font and lineage");return 0;
}
int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,matrix);}
