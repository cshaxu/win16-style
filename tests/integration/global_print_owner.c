#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global/session.h"
int wmain(int argc,WCHAR **argv)
{
    if(argc<4 || argc>6)return 2;
    /* The caller stages this executable as run16s.exe beside the tested DLLs.
     * The child installs the production receiver on its own thread only. */
    for(int i=3;i<argc;++i)
        if(!argv[i][0] || wcspbrk(argv[i],L" \t\""))return 2;
    WCHAR command[8192];
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --late \"%ls\" %ls %ls %ls",
        argv[1],argv[2],argv[3],argc>4?argv[4]:L"",argc>5?argv[5]:L"")<0)return 2;
    WCHAR name[NTW16S_GLOBAL_NAME_CAP];
    if(!style_global_session_name(&ntw16s_global_namespace,name,ARRAYSIZE(name)))return 3;
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!mapping)return 4;
    if(GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(mapping);puts("Late test skipped: desktop already has a session");return 77;}
    ntw16s_global_session *session=MapViewOfFile(mapping,FILE_MAP_WRITE,0,0,sizeof(*session));
    if(!session){CloseHandle(mapping);return 4;}
    *session=(ntw16s_global_session){.magic=NTW16S_GLOBAL_MAGIC,.version=NTW16S_GLOBAL_VERSION,
        .owner_pid=GetCurrentProcessId(),.cookie=GetCurrentProcessId()^0x84792831u,.style=NTW16S_GLOBAL_STYLE_2K,.ready=TRUE};
    if(!session->cookie)session->cookie=1;
    DWORD code=4;
    if(win32_process_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high)){
        InterlockedExchange(&session->phase,NTW16S_GLOBAL_ACTIVE);
        STARTUPINFOW start={.cb=sizeof(start)};
        PROCESS_INFORMATION child={0};
        if(CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&start,&child)){
            if(WaitForSingleObject(child.hProcess,24000)==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);
            else{TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,2000);code=6;}
            CloseHandle(child.hThread);
            CloseHandle(child.hProcess);
        }
    }
    InterlockedExchange(&session->phase,NTW16S_GLOBAL_STOPPING);
    UnmapViewOfFile(session);CloseHandle(mapping);
    printf("OWNED LATE DIALOG exit=%lu\n",code);
    return (int)code;
}
