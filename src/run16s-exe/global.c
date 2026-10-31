#include "global.h"
#include "ntw16s-dll/global_session.h"
#include "ntw16s-dll/injection/injection.h"
#include <windows.h>
#include <sddl.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct sweep_state {UINT message;DWORD cookie,host;LPARAM action;ULONGLONG deadline;unsigned attempted,failed;} sweep_state;
static HANDLE stop_signal;
static BOOL WINAPI stop_handler(DWORD reason)
{
    if(reason!=CTRL_C_EVENT && reason!=CTRL_BREAK_EVENT && reason!=CTRL_CLOSE_EVENT && reason!=CTRL_SHUTDOWN_EVENT)return FALSE;
    if(stop_signal)SetEvent(stop_signal);
    return TRUE;
}
static BOOL CALLBACK sweep_window(HWND w,LPARAM parameter)
{
    sweep_state *s=(sweep_state *)parameter;DWORD process=0;DWORD_PTR result=0;
    if(GetTickCount64()>=s->deadline)return FALSE;
    if(!GetWindowThreadProcessId(w,&process) || process==GetCurrentProcessId() || process==s->host)return TRUE;
    ++s->attempted;
    if(!SendMessageTimeoutW(w,s->message,s->cookie,s->action,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,50,&result))++s->failed;
    return TRUE;
}
static BOOL CALLBACK sweep_top(HWND w,LPARAM parameter)
{
    sweep_state *s=(sweep_state *)parameter;
    if(!sweep_window(w,parameter))return FALSE;
    EnumChildWindows(w,sweep_window,parameter);
    return GetTickCount64()<s->deadline;
}
static void sweep(UINT message,DWORD cookie,DWORD host,LPARAM action)
{
    sweep_state s={message,cookie,host,action,GetTickCount64()+5000,0,0};
    EnumWindows(sweep_top,(LPARAM)&s);
    fwprintf(stderr,L"run16s: global %ls: %u windows notified, %u timed out or failed%s\n",
        action==NTW16S_GLOBAL_APPLY?L"start":L"stop",s.attempted,s.failed,
        GetTickCount64()>=s.deadline?L"; time limit reached":L"");
}
static BOOL random_cookie(DWORD *value)
{
    HCRYPTPROV provider=0;BOOL ok=CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT|CRYPT_SILENT);
    if(ok){ok=CryptGenRandom(provider,sizeof(*value),(BYTE *)value);CryptReleaseContext(provider,0);}
    if(ok && !*value)*value=1;
    return ok;
}
static int fail(DWORD error,PCWSTR what)
{fwprintf(stderr,L"run16s: global %ls failed (%lu)\n",what,error);return 10;}
int run16s_global(void)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],ready_name[NTW16S_GLOBAL_NAME_CAP+16],host_stop_name[NTW16S_GLOBAL_NAME_CAP+16],request_name[NTW16S_GLOBAL_NAME_CAP+16];
    WCHAR wow[MAX_PATH],host_image[MAX_PATH],dll64[NTW16S_PATH_CAP],dll32[NTW16S_PATH_CAP],command[NTW16S_PATH_CAP+MAX_PATH+64];
    ntw16s_context *context=NULL;PSECURITY_DESCRIPTOR security=NULL;SECURITY_ATTRIBUTES attributes={sizeof(attributes),NULL,FALSE};
    HANDLE mapping=NULL,ready=NULL,host_stop=NULL;ntw16s_global_session *s=NULL;HMODULE dll=NULL;HHOOK hook=NULL;
    PROCESS_INFORMATION host={0};DWORD error=0;UINT message=0;int result=10;BOOL handler=FALSE;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)))return fail(GetLastError(),L"desktop identity");
    if(swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.ready",name)<0 || swprintf(host_stop_name,ARRAYSIZE(host_stop_name),L"%ls.stop",name)<0 ||
       swprintf(request_name,ARRAYSIZE(request_name),L"%ls.request",name)<0)return fail(ERROR_INSUFFICIENT_BUFFER,L"object name");
    if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:P(A;;GA;;;SY)(A;;GA;;;OW)",SDDL_REVISION_1,&security,NULL))return fail(GetLastError(),L"object security");
    attributes.lpSecurityDescriptor=security;
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,&attributes,PAGE_READWRITE,0,sizeof(*s),name);
    if(!mapping){error=GetLastError();goto done;}if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    s=(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*s));if(!s){error=GetLastError();goto done;}
    ZeroMemory(s,sizeof(*s));s->magic=NTW16S_GLOBAL_MAGIC;s->version=NTW16S_GLOBAL_VERSION;s->owner_pid=GetCurrentProcessId();
    if(!ntw16s_global_birth(GetCurrentProcess(),&s->owner_birth_low,&s->owner_birth_high) || !random_cookie(&s->cookie)){error=GetLastError();goto done;}
    ready=CreateEventW(&attributes,TRUE,FALSE,ready_name);if(!ready){error=GetLastError();goto done;}if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    host_stop=CreateEventW(&attributes,TRUE,FALSE,host_stop_name);if(!host_stop){error=GetLastError();goto done;}if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    stop_signal=CreateEventW(&attributes,TRUE,FALSE,request_name);if(!stop_signal){error=GetLastError();goto done;}
    if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    context=(ntw16s_context *)calloc(1,sizeof(*context));if(!context){error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    error=ntw16s_context_paths(context,NULL);if(error)goto done;
    if(swprintf(dll64,ARRAYSIZE(dll64),L"%lsntw16s64.dll",context->directory)<0 || swprintf(dll32,ARRAYSIZE(dll32),L"%lsntw16s32.dll",context->directory)<0){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    error=ntw16s_validate_image(dll64,IMAGE_FILE_MACHINE_AMD64,TRUE);if(error)goto done;
    error=ntw16s_validate_image(dll32,IMAGE_FILE_MACHINE_I386,TRUE);if(error)goto done;
    dll=LoadLibraryW(dll64);if(!dll){error=GetLastError();goto done;}
    HOOKPROC callback=(HOOKPROC)(void *)GetProcAddress(dll,"Ntw16sGlobalHook");if(!callback){error=GetLastError();goto done;}
    message=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);if(!message){error=GetLastError();goto done;}
    hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,dll,0);if(!hook){error=GetLastError();goto done;}
    if(!GetSystemWow64DirectoryW(wow,ARRAYSIZE(wow)) || swprintf(host_image,ARRAYSIZE(host_image),L"%ls\\rundll32.exe",wow)<0 ||
       swprintf(command,ARRAYSIZE(command),L"\"%ls\" \"%ls\",Ntw16sGlobalHost",host_image,dll32)<0){error=GetLastError();if(!error)error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    STARTUPINFOW startup={.cb=sizeof(startup)};
    if(!CreateProcessW(host_image,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&host)){error=GetLastError();goto done;}
    s->host_pid=host.dwProcessId;
    if(ResumeThread(host.hThread)==(DWORD)-1){error=GetLastError();goto done;}
    HANDLE startup_waits[2]={ready,host.hProcess};DWORD startup_status=WaitForMultipleObjects(2,startup_waits,FALSE,10000);
    if(startup_status!=WAIT_OBJECT_0){error=startup_status==WAIT_TIMEOUT?ERROR_TIMEOUT:ERROR_DLL_INIT_FAILED;goto done;}
    if(!SetConsoleCtrlHandler(stop_handler,TRUE)){error=GetLastError();goto done;}handler=TRUE;
    InterlockedExchange(&s->phase,NTW16S_GLOBAL_ACTIVE);
    sweep(message,s->cookie,s->host_pid,NTW16S_GLOBAL_APPLY);
    fwprintf(stderr,L"run16s: global active; press Ctrl+C to stop\n");
    for(;;){DWORD status=MsgWaitForMultipleObjects(1,&stop_signal,FALSE,INFINITE,QS_ALLINPUT);
        if(status==WAIT_OBJECT_0)break;
        if(status!=WAIT_OBJECT_0+1){error=GetLastError();goto done;}
        MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}
    }
    result=0;
done:
    if(s && message){InterlockedExchange(&s->phase,NTW16S_GLOBAL_STOPPING);if(hook)sweep(message,s->cookie,s->host_pid,NTW16S_GLOBAL_REMOVE);}
    if(hook)UnhookWindowsHookEx(hook);
    if(host_stop)SetEvent(host_stop);
    if(host.hProcess){if(WaitForSingleObject(host.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(host.hProcess,ERROR_TIMEOUT);WaitForSingleObject(host.hProcess,1000);if(!error)error=ERROR_TIMEOUT;}CloseHandle(host.hProcess);}
    if(host.hThread)CloseHandle(host.hThread);
    if(handler)SetConsoleCtrlHandler(stop_handler,FALSE);
    if(stop_signal){CloseHandle(stop_signal);stop_signal=NULL;}
    if(dll)FreeLibrary(dll);
    free(context);
    if(s)UnmapViewOfFile(s);
    if(mapping)CloseHandle(mapping);
    if(ready)CloseHandle(ready);
    if(host_stop)CloseHandle(host_stop);
    if(security)LocalFree(security);
    if(error)return fail(error,L"startup or shutdown");
    return result;
}
