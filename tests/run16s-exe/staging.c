#include "ntw16s-dll/injection/injection.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STAGING FAIL %d error=%lu\n",__LINE__,GetLastError());goto failed;}}while(0)
int wmain(int argc,WCHAR **argv)
{
    WCHAR self[NTW16S_PATH_CAP],command[NTW16S_PATH_CAP+4];ntw16s_context context;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};WORD machine;
    DWORD expected=sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;int result=1;
    if(argc==1)return 42;
    CHECK(argc==2 && !wcscmp(argv[1],L"--parent"));
    CHECK(!ntw16s_context_paths(&context,NULL));
    CHECK(GetModuleFileNameW(NULL,self,NTW16S_PATH_CAP));
    CHECK(!ntw16s_validate_image(self,(WORD)expected,FALSE));
    CHECK(ntw16s_validate_image(self,(WORD)expected,TRUE)==ERROR_BAD_EXE_FORMAT);
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\"",self)>0);
    CHECK(CreateProcessW(self,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&child));
    CHECK(!ntw16s_process_machine(child.hProcess,&machine) && machine==expected);
    CHECK(SuspendThread(child.hThread)==1 && ResumeThread(child.hThread)==2);
    context.flags=0x80000001u;
    CHECK(ntw16s_install(child.hProcess,&context,CreateProcessW)==ERROR_INVALID_DATA);
    context.flags=1;
    CHECK(!ntw16s_install(child.hProcess,&context,CreateProcessW));
    CHECK(SuspendThread(child.hThread)==1 && ResumeThread(child.hThread)==2);
    CHECK(WaitForSingleObject(child.hProcess,0)==WAIT_TIMEOUT);
    puts("STAGING PASS actual same-width target, payload/imports staged, original suspension retained; initialization not claimed");result=0;
failed:
    if(child.hProcess){TerminateProcess(child.hProcess,1);WaitForSingleObject(child.hProcess,2000);CloseHandle(child.hProcess);}
    if(child.hThread)CloseHandle(child.hThread);
    return result;
}
