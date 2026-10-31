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
        WCHAR dll[32752];
        if(!GetEnvironmentVariableW(L"WIN16_REMOTE_TEST_DLL",dll,ARRAYSIZE(dll)))return 7;
        HMODULE image=LoadLibraryW(dll);if(!image)return 7;
        WCHAR corrupt[2];
        if(GetEnvironmentVariableW(L"WIN16_REMOTE_TEST_CORRUPT",corrupt,ARRAYSIZE(corrupt))){
            IMAGE_DOS_HEADER *dos=(IMAGE_DOS_HEADER *)image;
            IMAGE_NT_HEADERS *nt=(IMAGE_NT_HEADERS *)((BYTE *)image+dos->e_lfanew);
            IMAGE_EXPORT_DIRECTORY *exports=(IMAGE_EXPORT_DIRECTORY *)((BYTE *)image+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
            DWORD old=0,ignored=0;
            if(!VirtualProtect(&exports->NumberOfNames,sizeof(exports->NumberOfNames),PAGE_EXECUTE_READWRITE,&old))return 9;
            exports->NumberOfNames=0xffffffffu;
            if(!VirtualProtect(&exports->NumberOfNames,sizeof(exports->NumberOfNames),old,&ignored))return 10;
        }
        HANDLE ready=event_from_env(L"WIN16_REMOTE_TEST_READY",EVENT_MODIFY_STATE);
        HANDLE stop=event_from_env(L"WIN16_REMOTE_TEST_STOP",SYNCHRONIZE);
        if(!ready || !stop)return 2;
        SetEvent(ready);DWORD result=WaitForSingleObject(stop,5000);CloseHandle(stop);CloseHandle(ready);
        return result==WAIT_OBJECT_0?0:3;
    }
    if(argc!=2)return 8;
    WCHAR desktop[128],ready_name[128],stop_name[128],self[MAX_PATH],cmd[MAX_PATH+32];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 4;
    swprintf(ready_name,ARRAYSIZE(ready_name),L"Local\\Win16RemoteReady-%lu",GetCurrentProcessId());
    swprintf(stop_name,ARRAYSIZE(stop_name),L"Local\\Win16RemoteStop-%lu",GetCurrentProcessId());
    SetEnvironmentVariableW(L"WIN16_REMOTE_TEST_READY",ready_name);SetEnvironmentVariableW(L"WIN16_REMOTE_TEST_STOP",stop_name);
    SetEnvironmentVariableW(L"WIN16_REMOTE_TEST_DLL",argv[1]);
    SetEnvironmentVariableW(L"WIN16_REMOTE_TEST_CORRUPT",NULL);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name),stop=CreateEventW(NULL,TRUE,FALSE,stop_name);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!ready || !stop || !CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))return 5;
    BOOL started=WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;void *remote=NULL,*invalid=(void *)1;
    DWORD error=started?ntw16s_remote_loadlibrary(pi.hProcess,pi.dwProcessId,&remote):ERROR_TIMEOUT;
    DWORD bad=ntw16s_remote_loadlibrary(pi.hProcess,pi.dwProcessId+1,&invalid);
    void *worker=NULL,*invalid_worker=(void *)1;
    DWORD worker_error=started?ntw16s_remote_global_thread(pi.hProcess,pi.dwProcessId,argv[1],&worker):ERROR_TIMEOUT;
    DWORD bad_name=ntw16s_remote_global_thread(pi.hProcess,pi.dwProcessId,L"C:\\not-project.dll",&invalid_worker);
    void *missing_worker=(void *)1;
    DWORD missing=ntw16s_remote_global_thread(pi.hProcess,pi.dwProcessId,
        sizeof(void*)==8?L"C:\\missing\\ntw16s64.dll":L"C:\\missing\\ntw16s32.dll",&missing_worker);
    BYTE local_bytes[16],remote_bytes[16];SIZE_T count=0;FARPROC local=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"LoadLibraryW");
    BOOL same=local && remote && ReadProcessMemory(GetCurrentProcess(),(void *)local,local_bytes,sizeof(local_bytes),&count) && count==sizeof(local_bytes) &&
        ReadProcessMemory(pi.hProcess,remote,remote_bytes,sizeof(remote_bytes),&count) && count==sizeof(remote_bytes) && !memcmp(local_bytes,remote_bytes,sizeof(local_bytes));
    HMODULE dll=LoadLibraryW(argv[1]);FARPROC local_worker=dll?GetProcAddress(dll,"Ntw16sGlobalThread"):NULL;
    BOOL worker_same=local_worker && worker && ReadProcessMemory(GetCurrentProcess(),(void *)local_worker,local_bytes,sizeof(local_bytes),&count) && count==sizeof(local_bytes) &&
        ReadProcessMemory(pi.hProcess,worker,remote_bytes,sizeof(remote_bytes),&count) && count==sizeof(remote_bytes) && !memcmp(local_bytes,remote_bytes,sizeof(local_bytes));
    if(dll)FreeLibrary(dll);
    SetEvent(stop);DWORD wait=WaitForSingleObject(pi.hProcess,3000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
    ResetEvent(ready);ResetEvent(stop);SetEnvironmentVariableW(L"WIN16_REMOTE_TEST_CORRUPT",L"1");
    STARTUPINFOW corrupt_si={.cb=sizeof(corrupt_si)};PROCESS_INFORMATION corrupt_pi={0};
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    if(!CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&corrupt_si,&corrupt_pi))return 11;
    BOOL corrupt_ready=WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;void *corrupt_worker=(void *)1;
    DWORD corrupt_error=corrupt_ready?ntw16s_remote_global_thread(corrupt_pi.hProcess,corrupt_pi.dwProcessId,argv[1],&corrupt_worker):ERROR_TIMEOUT;
    SetEvent(stop);DWORD corrupt_wait=WaitForSingleObject(corrupt_pi.hProcess,3000),corrupt_code=99;
    if(corrupt_wait==WAIT_OBJECT_0)GetExitCodeProcess(corrupt_pi.hProcess,&corrupt_code);else TerminateProcess(corrupt_pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(corrupt_pi.hThread);CloseHandle(corrupt_pi.hProcess);CloseHandle(ready);CloseHandle(stop);
    if(error || !same || worker_error || !worker_same || bad!=ERROR_INVALID_PARAMETER || invalid || bad_name!=ERROR_INVALID_NAME || invalid_worker || missing!=ERROR_MOD_NOT_FOUND || missing_worker || wait!=WAIT_OBJECT_0 || code || corrupt_error!=ERROR_BAD_EXE_FORMAT || corrupt_worker || corrupt_wait!=WAIT_OBJECT_0 || corrupt_code){printf("REMOTE RESOLVE FAIL error=%lu worker_error=%lu bad=%lu bad_name=%lu missing=%lu corrupt=%lu same=%d worker=%d wait=%lu code=%lu\n",error,worker_error,bad,bad_name,missing,corrupt_error,same,worker_same,wait,code);return 6;}
    printf("REMOTE RESOLVE PASS bits=%u loader=1 worker=1 bad_pid=1 bad_name=1 missing=1 corrupt=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
