#include "lib-win32/process-image/image.h"
#include "global.h"
#include "elevation.h"
#include "lib-win32/security/elevation.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/global/manager.h"
#include "ntw16s-dll/global/process.h"
#include "lib-win32/process-global/window.h"
#include "lib-win32/process-global/identity.h"
#include "ntwshk-dll/injection.h"
#include <windows.h>
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
static BOOL sweep_window(HWND w,void *parameter)
{
    sweep_state *s=(sweep_state *)parameter;DWORD process=0;DWORD_PTR result=0;
    if(!GetWindowThreadProcessId(w,&process) || process==GetCurrentProcessId() || process==s->host)return TRUE;
    ++s->attempted;
    if(!SendMessageTimeoutW(w,s->message,s->cookie,s->action,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,50,&result)){
        DWORD error=GetLastError();ULONGLONG now=GetTickCount64();
        DWORD retry=now<s->deadline?(DWORD)min(200,s->deadline-now):0;
        if(error!=ERROR_TIMEOUT || !retry ||
           !SendMessageTimeoutW(w,s->message,s->cookie,s->action,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,retry,&result))++s->failed;
    }
    return TRUE;
}
static DWORD sweep(UINT message,DWORD cookie,DWORD host,LPARAM action)
{
    sweep_state s={message,cookie,host,action,GetTickCount64()+5000,0,0};
    DWORD error=win32_process_global_window_walk(0,s.deadline,sweep_window,&s)?ERROR_SUCCESS:GetLastError();
    fwprintf(stderr,L"run16s: global %ls: %u windows notified, %u timed out or failed%s\n",
        action==NTW16S_GLOBAL_APPLY?L"start":L"stop",s.attempted,s.failed,
        GetTickCount64()>=s.deadline?L"; time limit reached":L"");
    return error==ERROR_TIMEOUT?ERROR_SUCCESS:error;
}
static DWORD random_cookie(DWORD *value)
{
    HCRYPTPROV provider=0;
    if(!CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT|CRYPT_SILENT))return GetLastError();
    DWORD error=CryptGenRandom(provider,sizeof(*value),(BYTE *)value)?ERROR_SUCCESS:GetLastError();
    if(!CryptReleaseContext(provider,0) && !error)error=GetLastError();
    if(!error && !*value)*value=1;
    return error;
}
static int fail(DWORD error,PCWSTR what)
{fwprintf(stderr,L"run16s: global %ls failed (%lu)\n",what,error);return 10;}
int run16s_global_with_parent(HANDLE parent,DWORD style)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],ready_name[NTW16S_GLOBAL_NAME_CAP+16],host_stop_name[NTW16S_GLOBAL_NAME_CAP+16],request_name[NTW16S_GLOBAL_NAME_CAP+16];
    WCHAR wow[MAX_PATH],host_image[MAX_PATH],dll64[NTWSHK_PATH_CAP],dll32[NTWSHK_PATH_CAP],command[NTWSHK_PATH_CAP+MAX_PATH+64];
    ntwshk_context *context=NULL;PSECURITY_DESCRIPTOR security=NULL;SECURITY_ATTRIBUTES attributes={sizeof(attributes),NULL,FALSE};
    HANDLE mapping=NULL,ready=NULL,host_stop=NULL;ntw16s_global_session *s=NULL;HMODULE dll=NULL;HHOOK hook=NULL;
    PROCESS_INFORMATION host={0};DWORD error=0;UINT message=0;int result=10;BOOL handler=FALSE;
    ntw16s_global_manager *manager=NULL;
    if(style!=NTW16S_GLOBAL_STYLE_16 && style!=NTW16S_GLOBAL_STYLE_2K)return fail(ERROR_NOT_SUPPORTED,L"style");
    if(parent){DWORD state=WaitForSingleObject(parent,0);if(state==WAIT_OBJECT_0)return 0;
        if(state!=WAIT_TIMEOUT)return fail(state==WAIT_FAILED?GetLastError():ERROR_INVALID_HANDLE,L"parent wait");}
    DWORD elevation_error=0;
    if(!win32_security_is_elevated(&elevation_error)){
        if(elevation_error)fwprintf(stderr,L"run16s: unable to check administrator token (%lu)\n",elevation_error);
        else fwprintf(stderr,L"run16s: global background process events require administrator privileges; window discovery remains active\n");
    }
    if(!ntw16s_global_name(name,ARRAYSIZE(name)))return fail(GetLastError(),L"desktop identity");
    if(swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.ready",name)<0 || swprintf(host_stop_name,ARRAYSIZE(host_stop_name),L"%ls.stop",name)<0 ||
       swprintf(request_name,ARRAYSIZE(request_name),L"%ls.request",name)<0)return fail(ERROR_INSUFFICIENT_BUFFER,L"object name");
    error=win32_process_global_user_security(FALSE,&security);if(error)return fail(error,L"object security");
    attributes.lpSecurityDescriptor=security;
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,&attributes,PAGE_READWRITE,0,sizeof(*s),name);
    if(!mapping){error=GetLastError();goto done;}if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    s=(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*s));if(!s){error=GetLastError();goto done;}
    ZeroMemory(s,sizeof(*s));s->magic=NTW16S_GLOBAL_MAGIC;s->version=NTW16S_GLOBAL_VERSION;s->owner_pid=GetCurrentProcessId();
    s->style=style;
    if(!win32_process_global_birth(GetCurrentProcess(),&s->owner_birth_low,&s->owner_birth_high)){error=GetLastError();goto done;}
    error=random_cookie(&s->cookie);if(error)goto done;
    ready=CreateEventW(&attributes,TRUE,FALSE,ready_name);if(!ready){error=GetLastError();goto done;}if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    host_stop=CreateEventW(&attributes,TRUE,FALSE,host_stop_name);if(!host_stop){error=GetLastError();goto done;}if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    stop_signal=CreateEventW(&attributes,TRUE,FALSE,request_name);if(!stop_signal){error=GetLastError();goto done;}
    if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    /* Startup may include a process snapshot. Honour Ctrl+C during that work
     * and reach the same orderly cleanup path as an active session. */
    if(!SetConsoleCtrlHandler(stop_handler,TRUE)){error=GetLastError();goto done;}handler=TRUE;
    context=(ntwshk_context *)calloc(1,sizeof(*context));if(!context){error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    error=ntwshk_context_paths(context,NULL);if(error)goto done;
    if(style==NTW16S_GLOBAL_STYLE_2K){
        if(swprintf(dll64,ARRAYSIZE(dll64),L"%lsntw2ks64.dll",context->directory)<0 ||
           swprintf(dll32,ARRAYSIZE(dll32),L"%lsntw2ks32.dll",context->directory)<0){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
        error=win32_process_image_validate(dll64,IMAGE_FILE_MACHINE_AMD64,TRUE);if(error)goto done;
        error=win32_process_image_validate(dll32,IMAGE_FILE_MACHINE_I386,TRUE);if(error)goto done;
    }
    if(swprintf(dll64,ARRAYSIZE(dll64),L"%lsntw16s64.dll",context->directory)<0 || swprintf(dll32,ARRAYSIZE(dll32),L"%lsntw16s32.dll",context->directory)<0){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    error=win32_process_image_validate(dll64,IMAGE_FILE_MACHINE_AMD64,TRUE);if(error)goto done;
    error=win32_process_image_validate(dll32,IMAGE_FILE_MACHINE_I386,TRUE);if(error)goto done;
    dll=LoadLibraryW(dll64);if(!dll){error=GetLastError();goto done;}
    HOOKPROC callback=(HOOKPROC)(void *)GetProcAddress(dll,"Ntw16sGlobalHook");if(!callback){error=GetLastError();goto done;}
    message=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);if(!message){error=GetLastError();goto done;}
    hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,dll,0);
    if(!hook){error=GetLastError();if(error!=ERROR_MOD_NOT_FOUND)goto done;error=0;
        fwprintf(stderr,L"run16s: global x64 hook unavailable (126); using target-thread fallback\n");}
    /* The fallback host needs an active session before it begins discovery. */
    InterlockedExchange(&s->phase,NTW16S_GLOBAL_ACTIVE);
    if(!GetSystemWow64DirectoryW(wow,ARRAYSIZE(wow)) || swprintf(host_image,ARRAYSIZE(host_image),L"%ls\\rundll32.exe",wow)<0 ||
       swprintf(command,ARRAYSIZE(command),L"\"%ls\" \"%ls\",Ntw16sGlobalHost",host_image,dll32)<0){error=GetLastError();if(!error)error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    STARTUPINFOW startup={.cb=sizeof(startup)};
    if(!CreateProcessW(host_image,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&host)){error=GetLastError();goto done;}
    s->host_pid=host.dwProcessId;
    if(ResumeThread(host.hThread)==(DWORD)-1){error=GetLastError();goto done;}
    HANDLE startup_waits[3]={ready,host.hProcess,parent};DWORD startup_status=WaitForMultipleObjects(parent?3:2,startup_waits,FALSE,10000);
    if(parent && startup_status==WAIT_OBJECT_0+2){result=0;goto done;}
    if(startup_status!=WAIT_OBJECT_0){
        if(startup_status==WAIT_OBJECT_0+1){DWORD host_code=0;
            if(GetExitCodeProcess(host.hProcess,&host_code))fwprintf(stderr,L"run16s: global 32-bit host exited (%lu)\n",host_code);}
        error=startup_status==WAIT_FAILED?GetLastError():startup_status==WAIT_TIMEOUT?ERROR_TIMEOUT:
            startup_status==WAIT_OBJECT_0+1?ERROR_DLL_INIT_FAILED:ERROR_INVALID_STATE;
        goto done;
    }
    error=ntw16s_global_manager_start(dll64,IMAGE_FILE_MACHINE_AMD64,&manager);if(error)goto done;
    error=sweep(message,s->cookie,s->host_pid,NTW16S_GLOBAL_APPLY);if(error)goto done;
    /* A target may have appeared while the startup sweep was sending
     * messages. Revisit it before announcing that discovery is ready. */
    error=ntw16s_global_manager_refresh(manager);if(error)goto done;
    error=ntw16s_global_manager_wait_idle(manager,2000);
    if(error==ERROR_TIMEOUT){fwprintf(stderr,L"run16s: global startup attachment still pending\n");error=0;}
    if(error)goto done;
    if(parent && WaitForSingleObject(parent,0)==WAIT_OBJECT_0){result=0;goto done;}
    InterlockedExchange(&s->ready,1);
    fwprintf(stderr,L"run16s: global active; press Ctrl+C to stop\n");
    HANDLE active_waits[3]={stop_signal,host.hProcess,parent};DWORD active_count=parent?3:2;
    for(;;){DWORD status=MsgWaitForMultipleObjects(active_count,active_waits,FALSE,INFINITE,QS_ALLINPUT);
        if(status==WAIT_OBJECT_0)break;
        if(status==WAIT_OBJECT_0+1){error=ERROR_DLL_INIT_FAILED;goto done;}
        if(parent && status==WAIT_OBJECT_0+2)break;
        if(status!=WAIT_OBJECT_0+active_count){error=GetLastError();goto done;}
        MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}
    }
    result=0;
done:
    if(s && message){InterlockedExchange(&s->ready,0);InterlockedExchange(&s->phase,NTW16S_GLOBAL_STOPPING);
        DWORD sweep_error=sweep(message,s->cookie,s->host_pid,NTW16S_GLOBAL_REMOVE);if(sweep_error && !error)error=sweep_error;}
    if(hook)UnhookWindowsHookEx(hook);
    if(host_stop)SetEvent(host_stop);
    if(manager){DWORD stopped=ntw16s_global_manager_stop(manager,5000);if(stopped && !error)error=stopped;}
    if(host.hProcess){if(WaitForSingleObject(host.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(host.hProcess,ERROR_TIMEOUT);WaitForSingleObject(host.hProcess,1000);if(!error)error=ERROR_TIMEOUT;}
        if(WaitForSingleObject(host.hProcess,0)==WAIT_OBJECT_0){
            DWORD cleanup=ntw16s_global_process_stop_peer(host.hProcess,host.dwProcessId);
            if(cleanup && cleanup!=ERROR_ACCESS_DENIED)fwprintf(stderr,L"run16s: global peer trace cleanup failed (%lu)\n",cleanup);
        }
        CloseHandle(host.hProcess);}
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
