#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "ntw16s-dll/global_remote.h"

static HANDLE event_from_env(const WCHAR *variable,DWORD access)
{
    WCHAR name[128];
    return GetEnvironmentVariableW(variable,name,ARRAYSIZE(name))?OpenEventW(access,FALSE,name):NULL;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target")){
        HANDLE ready=event_from_env(L"WIN16_REMOTE_TEST_READY",EVENT_MODIFY_STATE);
        HANDLE stop=event_from_env(L"WIN16_REMOTE_TEST_STOP",SYNCHRONIZE);
        if(!ready || !stop)return 2;
        SetEvent(ready);DWORD result=WaitForSingleObject(stop,5000);CloseHandle(stop);CloseHandle(ready);
        return result==WAIT_OBJECT_0?0:3;
    }
    WCHAR desktop[128],ready_name[128],stop_name[128],self[MAX_PATH],cmd[MAX_PATH+32];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 4;
    swprintf(ready_name,ARRAYSIZE(ready_name),L"Local\\Win16RemoteReady-%lu",GetCurrentProcessId());
    swprintf(stop_name,ARRAYSIZE(stop_name),L"Local\\Win16RemoteStop-%lu",GetCurrentProcessId());
    SetEnvironmentVariableW(L"WIN16_REMOTE_TEST_READY",ready_name);SetEnvironmentVariableW(L"WIN16_REMOTE_TEST_STOP",stop_name);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name),stop=CreateEventW(NULL,TRUE,FALSE,stop_name);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!ready || !stop || !CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))return 5;
    BOOL started=WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;void *remote=NULL,*invalid=(void *)1;
    DWORD error=started?ntw16s_remote_loadlibrary(pi.hProcess,pi.dwProcessId,&remote):ERROR_TIMEOUT;
    DWORD bad=ntw16s_remote_loadlibrary(pi.hProcess,pi.dwProcessId+1,&invalid);
    BYTE local_bytes[16],remote_bytes[16];SIZE_T count=0;FARPROC local=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"LoadLibraryW");
    BOOL same=local && remote && ReadProcessMemory(GetCurrentProcess(),(void *)local,local_bytes,sizeof(local_bytes),&count) && count==sizeof(local_bytes) &&
        ReadProcessMemory(pi.hProcess,remote,remote_bytes,sizeof(remote_bytes),&count) && count==sizeof(remote_bytes) && !memcmp(local_bytes,remote_bytes,sizeof(local_bytes));
    SetEvent(stop);DWORD wait=WaitForSingleObject(pi.hProcess,3000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(ready);CloseHandle(stop);
    if(error || !same || bad!=ERROR_INVALID_PARAMETER || invalid || wait!=WAIT_OBJECT_0 || code){printf("REMOTE RESOLVE FAIL error=%lu bad=%lu same=%d wait=%lu code=%lu\n",error,bad,same,wait,code);return 6;}
    printf("REMOTE RESOLVE PASS bits=%u header_prefix=1 bad_pid=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
