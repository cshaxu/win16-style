#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"

static BOOL launch(const WCHAR *image,WCHAR *command,WCHAR *desktop,PROCESS_INFORMATION *p)
{STARTUPINFOW si={.cb=sizeof(si),.lpDesktop=desktop};return CreateProcessW(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,p);}
int wmain(int argc,WCHAR **argv)
{
    if(argc==6 && !wcscmp(argv[1],L"broker")){
        HANDLE go=OpenEventW(SYNCHRONIZE,FALSE,argv[3]);if(!go)return 2;
        if(WaitForSingleObject(go,10000)!=WAIT_OBJECT_0){CloseHandle(go);return 3;}CloseHandle(go);
        BOOL loaded=GetModuleHandleW(L"ntw16s64.dll")!=NULL;WCHAR cmd[32768];PROCESS_INFORMATION target={0};
        if(swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls",argv[4],argv[5])<0 || !launch(argv[4],cmd,argv[2],&target))return 4;
        DWORD code=99;if(WaitForSingleObject(target.hProcess,8000)==WAIT_OBJECT_0)GetExitCodeProcess(target.hProcess,&code);
        else{TerminateProcess(target.hProcess,ERROR_TIMEOUT);WaitForSingleObject(target.hProcess,1000);}
        CloseHandle(target.hThread);CloseHandle(target.hProcess);
        printf("CROSS BROKER loaded=%d target_exit=%lu\n",loaded,code);return !loaded && !code?0:1;
    }
    if(argc!=4)return 5;
    WCHAR desktop[128],other[128],event_name[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16],self[32768],cmd[32768];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14) || !ntw16s_global_name(name,ARRAYSIZE(name)))return 6;
    swprintf(other,ARRAYSIZE(other),L"Win16S-Origin-%lu",GetCurrentProcessId());
    swprintf(event_name,ARRAYSIZE(event_name),L"Local\\Win16S-Origin-Go-%lu",GetCurrentProcessId());
    swprintf(request,ARRAYSIZE(request),L"%ls.request",name);
    HDESK sibling=CreateDesktopW(other,NULL,NULL,0,GENERIC_ALL,NULL);
    HANDLE go=CreateEventW(NULL,TRUE,FALSE,event_name),mapping=NULL,stop=NULL;ntw16s_global_session *session=NULL;
    PROCESS_INFORMATION broker={0},manager={0};BOOL pass=FALSE;DWORD code=99;
    if(!sibling || !go || !GetModuleFileNameW(NULL,self,ARRAYSIZE(self)))goto done;
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" broker \"%ls\" %ls \"%ls\" %ls",self,desktop,event_name,argv[2],argv[3]);
    if(!launch(self,cmd,other,&broker))goto done;
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" --global",argv[1]);if(!launch(argv[1],cmd,NULL,&manager))goto done;
    for(unsigned i=0;i<250;i++){
        if(!mapping)mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        if(mapping && !session)session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));
        if(session && session->owner_pid==manager.dwProcessId && session->phase==NTW16S_GLOBAL_ACTIVE && session->ready)break;
        if(WaitForSingleObject(manager.hProcess,0)!=WAIT_TIMEOUT)break;
        Sleep(20);
    }
    if(!session || session->owner_pid!=manager.dwProcessId || !session->ready || !SetEvent(go))goto done;
    if(WaitForSingleObject(broker.hProcess,10000)==WAIT_OBJECT_0 && GetExitCodeProcess(broker.hProcess,&code) && !code)pass=TRUE;
    printf("CROSS RESULT mode=%ls broker_exit=%lu pass=%d\n",argv[3],code,pass);
done:
    if(manager.hProcess && WaitForSingleObject(manager.hProcess,0)==WAIT_TIMEOUT){stop=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(stop)SetEvent(stop);if(WaitForSingleObject(manager.hProcess,6000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
    if(broker.hProcess && WaitForSingleObject(broker.hProcess,0)==WAIT_TIMEOUT){TerminateProcess(broker.hProcess,ERROR_TIMEOUT);WaitForSingleObject(broker.hProcess,1000);}
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(stop)CloseHandle(stop);
    if(go)CloseHandle(go);
    if(broker.hThread)CloseHandle(broker.hThread);
    if(broker.hProcess)CloseHandle(broker.hProcess);
    if(manager.hThread)CloseHandle(manager.hThread);
    if(manager.hProcess)CloseHandle(manager.hProcess);
    if(sibling)CloseDesktop(sibling);
    return pass?0:1;
}
