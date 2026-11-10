#include "ntw16s-dll/injection/injection.h"
#include <stdio.h>
#include <wchar.h>

int wmain(int argc,WCHAR **argv)
{
    if(argc!=3 && argc!=4)return 1;
    ntw16s_context context={0};
    if(wcslen(argv[2])>=NTW16S_PATH_CAP)return 2;
    wcscpy(context.directory,argv[2]);
    context.flags=NTW16S_POLICY_STYLE|NTW16S_POLICY_WIN2K|(argc==4?NTW16S_POLICY_PROPAGATE:0);
    WCHAR command[32768];
    if(argc==4 && swprintf(command,32768,L"\"%ls\" \"%ls\"",argv[1],argv[3])<0)return 2;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    if(!CreateProcessW(argv[1],argc==4?command:NULL,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&child))return 3;
    DWORD error=ntw16s_install(child.hProcess,&context,CreateProcessW),code=1;
    if(!error && ResumeThread(child.hThread)!=1)error=ERROR_INVALID_STATE;
    if(!error && WaitForSingleObject(child.hProcess,10000)!=WAIT_OBJECT_0)error=ERROR_TIMEOUT;
    if(!error && !GetExitCodeProcess(child.hProcess,&code))error=GetLastError();
    if(error){TerminateProcess(child.hProcess,error);WaitForSingleObject(child.hProcess,5000);}
    CloseHandle(child.hThread);CloseHandle(child.hProcess);
    if(error || code){fprintf(stderr,"NTW2KS STAGING FAIL error=%lu target=%lu\n",error,code);return 4;}
    printf("NTW2KS STAGING PASS selected=win2k win16_absent=1 propagated=%d\n",argc==4);return 0;
}
