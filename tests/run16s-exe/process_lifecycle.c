#include "run16s-exe/process_lifecycle.h"
#include "lib-win32/process-launch/process.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PROCESS_LIFECYCLE_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static DWORD prepare(HANDLE process,void *context)
{(void)process;return *(DWORD *)context;}
static BOOL start(PROCESS_INFORMATION *child)
{
    WCHAR self[MAX_PATH],command[MAX_PATH+16];STARTUPINFOW startup={.cb=sizeof(startup)};
    if(!GetModuleFileNameW(NULL,self,MAX_PATH))return FALSE;
    if(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" child",self)<0)return FALSE;
    return win32_process_create(self,command,NULL,NULL,FALSE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&startup,child);
}
int main(int argc,char **argv)
{
    if(argc==2 && !strcmp(argv[1],"child"))return 42;
    PROCESS_INFORMATION child={0};DWORD result=0,code=0;unsigned phase=0;HANDLE observed=NULL;
    CHECK(win32_process_activate(NULL,prepare,&result,&phase)==ERROR_INVALID_PARAMETER);
    CHECK(win32_process_wait(NULL,&code)==ERROR_INVALID_PARAMETER);
    CHECK(start(&child));CHECK(!win32_process_activate(&child,prepare,&result,&phase));
    CHECK(!child.hThread && child.hProcess);CHECK(!win32_process_wait(child.hProcess,&code) && code==42);CloseHandle(child.hProcess);
    CHECK(start(&child));CHECK(DuplicateHandle(GetCurrentProcess(),child.hProcess,GetCurrentProcess(),&observed,0,FALSE,DUPLICATE_SAME_ACCESS));
    result=ERROR_ACCESS_DENIED;CHECK(win32_process_activate(&child,prepare,&result,&phase)==ERROR_ACCESS_DENIED);
    CHECK(phase==WIN32_PROCESS_PREPARE_FAILED && !child.hProcess && !child.hThread);
    CHECK(WaitForSingleObject(observed,5000)==WAIT_OBJECT_0);
    CHECK(GetExitCodeProcess(observed,&code) && code==ERROR_ACCESS_DENIED);CloseHandle(observed);
    CHECK(start(&child));CHECK(DuplicateHandle(GetCurrentProcess(),child.hProcess,GetCurrentProcess(),&observed,0,FALSE,DUPLICATE_SAME_ACCESS));
    CHECK(SuspendThread(child.hThread)==1);result=0;
    CHECK(win32_process_activate(&child,prepare,&result,&phase)==ERROR_INVALID_STATE);
    CHECK(phase==WIN32_PROCESS_RESUME_FAILED && !child.hProcess && !child.hThread);
    CHECK(WaitForSingleObject(observed,5000)==WAIT_OBJECT_0);
    CHECK(GetExitCodeProcess(observed,&code) && code==ERROR_INVALID_STATE);CloseHandle(observed);
    puts("PROCESS_LIFECYCLE_PASS success=1 prepare_failure=1 resume_failure=1 exited=1");return 0;
}
