#include "owner.h"
#include "trace.h"
#include "lib-win32/process-global/window.h"
#include "lib-win32/process-global/identity.h"
#include "lib-win32-style/access/startup/context.h"
#include <wincrypt.h>
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
static DWORD sweep(const style_global_owner_spec *spec,UINT message,DWORD cookie,DWORD host,LPARAM action)
{
    sweep_state s={message,cookie,host,action,GetTickCount64()+5000,0,0};
    DWORD error=win32_process_global_window_walk(0,s.deadline,sweep_window,&s)?ERROR_SUCCESS:GetLastError();
    if(spec->report)spec->report(&(style_global_owner_notice){.kind=STYLE_GLOBAL_OWNER_SWEEP,
        .applying=action==spec->manager->attach->apply,.attempted=s.attempted,.failed=s.failed,
        .deadline=GetTickCount64()>=s.deadline},spec->context);
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
DWORD style_global_owner_run(const style_global_owner_spec *spec,HANDLE parent,DWORD style,style_global_manager **retained)
{
    *retained=NULL;
    WCHAR name[STYLE_GLOBAL_NAME_CAP],ready_name[STYLE_GLOBAL_NAME_CAP+16],host_stop_name[STYLE_GLOBAL_NAME_CAP+16],request_name[STYLE_GLOBAL_NAME_CAP+16];
    WCHAR wow[MAX_PATH],host_image[MAX_PATH],dll64[STYLE_STARTUP_PATH_CAP],dll32[STYLE_STARTUP_PATH_CAP],command[STYLE_STARTUP_PATH_CAP+MAX_PATH+64];
    PSECURITY_DESCRIPTOR security=NULL;SECURITY_ATTRIBUTES attributes={sizeof(attributes),NULL,FALSE};
    HANDLE mapping=NULL,ready=NULL,host_stop=NULL;style_global_session *s=NULL;HMODULE dll=NULL;HHOOK hook=NULL;
    PROCESS_INFORMATION host={0};DWORD error=0;UINT message=0;BOOL completed=FALSE,handler=FALSE;
    style_global_manager *manager=NULL;
    if(parent){DWORD state=WaitForSingleObject(parent,0);if(state==WAIT_OBJECT_0)return ERROR_SUCCESS;
        if(state!=WAIT_TIMEOUT)return state==WAIT_FAILED?GetLastError():ERROR_INVALID_HANDLE;}
    if(!style_global_session_name(spec->manager->attach->start->space,name,ARRAYSIZE(name)))return GetLastError();
    if(swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.ready",name)<0 || swprintf(host_stop_name,ARRAYSIZE(host_stop_name),L"%ls.stop",name)<0 ||
       swprintf(request_name,ARRAYSIZE(request_name),L"%ls.request",name)<0)return ERROR_INSUFFICIENT_BUFFER;
    error=win32_process_global_user_security(FALSE,&security);if(error)return error;
    attributes.lpSecurityDescriptor=security;
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,&attributes,PAGE_READWRITE,0,sizeof(*s),name);
    if(!mapping){error=GetLastError();goto done;}if(GetLastError()==ERROR_ALREADY_EXISTS){error=ERROR_ALREADY_EXISTS;goto done;}
    s=(style_global_session *)MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*s));if(!s){error=GetLastError();goto done;}
    ZeroMemory(s,sizeof(*s));s->magic=spec->manager->attach->start->space->magic;s->version=spec->manager->attach->start->space->version;s->owner_pid=GetCurrentProcessId();
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
    error=spec->paths(style,dll64,dll32,ARRAYSIZE(dll64),spec->context);if(error)goto done;
    dll=LoadLibraryW(dll64);if(!dll){error=GetLastError();goto done;}
    HOOKPROC callback=(HOOKPROC)(void *)GetProcAddress(dll,spec->hook_export);if(!callback){error=GetLastError();goto done;}
    message=RegisterWindowMessageW(spec->manager->attach->control);if(!message){error=GetLastError();goto done;}
    hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,dll,0);
    if(!hook){error=GetLastError();if(error!=ERROR_MOD_NOT_FOUND)goto done;error=0;
        if(spec->report)spec->report(&(style_global_owner_notice){.kind=STYLE_GLOBAL_OWNER_HOOK_FALLBACK},spec->context);}
    /* The fallback host needs an active session before it begins discovery. */
    InterlockedExchange(&s->phase,STYLE_GLOBAL_ACTIVE);
    if(!GetSystemWow64DirectoryW(wow,ARRAYSIZE(wow)) || swprintf(host_image,ARRAYSIZE(host_image),L"%ls\\rundll32.exe",wow)<0 ||
       swprintf(command,ARRAYSIZE(command),L"\"%ls\" \"%ls\",%ls",host_image,dll32,spec->host_export)<0){error=GetLastError();if(!error)error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    STARTUPINFOW startup={.cb=sizeof(startup)};
    if(!CreateProcessW(host_image,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&host)){error=GetLastError();goto done;}
    s->host_pid=host.dwProcessId;
    if(ResumeThread(host.hThread)==(DWORD)-1){error=GetLastError();goto done;}
    HANDLE startup_waits[3]={ready,host.hProcess,parent};DWORD startup_status=WaitForMultipleObjects(parent?3:2,startup_waits,FALSE,10000);
    if(parent && startup_status==WAIT_OBJECT_0+2){completed=TRUE;goto done;}
    if(startup_status!=WAIT_OBJECT_0){
        if(startup_status==WAIT_OBJECT_0+1){DWORD host_code=0;
            if(GetExitCodeProcess(host.hProcess,&host_code) && spec->report)spec->report(&(style_global_owner_notice){.kind=STYLE_GLOBAL_OWNER_HOST_EXIT,.error=host_code},spec->context);}
        error=startup_status==WAIT_FAILED?GetLastError():startup_status==WAIT_TIMEOUT?ERROR_TIMEOUT:
            startup_status==WAIT_OBJECT_0+1?ERROR_DLL_INIT_FAILED:ERROR_INVALID_STATE;
        goto done;
    }
    error=style_global_manager_start(spec->manager,dll64,IMAGE_FILE_MACHINE_AMD64,&manager);if(error)goto done;
    error=sweep(spec,message,s->cookie,s->host_pid,spec->manager->attach->apply);if(error)goto done;
    /* A target may have appeared while the startup sweep was sending
     * messages. Revisit it before announcing that discovery is ready. */
    error=style_global_manager_refresh(manager);if(error)goto done;
    error=style_global_manager_wait_idle(manager,2000);
    if(error==ERROR_TIMEOUT){if(spec->report)spec->report(&(style_global_owner_notice){.kind=STYLE_GLOBAL_OWNER_PENDING},spec->context);error=0;}
    if(error)goto done;
    if(parent && WaitForSingleObject(parent,0)==WAIT_OBJECT_0){completed=TRUE;goto done;}
    InterlockedExchange(&s->ready,1);
    if(spec->report)spec->report(&(style_global_owner_notice){.kind=STYLE_GLOBAL_OWNER_ACTIVE},spec->context);
    HANDLE active_waits[3]={stop_signal,host.hProcess,parent};DWORD active_count=parent?3:2;
    for(;;){DWORD status=MsgWaitForMultipleObjects(active_count,active_waits,FALSE,INFINITE,QS_ALLINPUT);
        if(status==WAIT_OBJECT_0)break;
        if(status==WAIT_OBJECT_0+1){error=ERROR_DLL_INIT_FAILED;goto done;}
        if(parent && status==WAIT_OBJECT_0+2)break;
        if(status!=WAIT_OBJECT_0+active_count){error=GetLastError();goto done;}
        MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}
    }
    completed=TRUE;
done:
    if(s && message){InterlockedExchange(&s->ready,0);InterlockedExchange(&s->phase,STYLE_GLOBAL_STOPPING);
        DWORD sweep_error=sweep(spec,message,s->cookie,s->host_pid,spec->manager->attach->remove);if(sweep_error && !error)error=sweep_error;}
    if(hook)UnhookWindowsHookEx(hook);
    if(host_stop)SetEvent(host_stop);
    if(manager){DWORD stopped=style_global_manager_stop(manager,5000);if(stopped){*retained=manager;if(!error)error=stopped;}}
    if(host.hProcess){if(WaitForSingleObject(host.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(host.hProcess,ERROR_TIMEOUT);WaitForSingleObject(host.hProcess,1000);if(!error)error=ERROR_TIMEOUT;}
        if(WaitForSingleObject(host.hProcess,0)==WAIT_OBJECT_0){
            DWORD cleanup=style_global_trace_stop_peer(spec->manager->trace_prefix,host.hProcess,host.dwProcessId);
            if(cleanup && cleanup!=ERROR_ACCESS_DENIED && spec->report)spec->report(&(style_global_owner_notice){.kind=STYLE_GLOBAL_OWNER_PEER_TRACE,.error=cleanup},spec->context);
        }
        CloseHandle(host.hProcess);}
    if(host.hThread)CloseHandle(host.hThread);
    if(handler)SetConsoleCtrlHandler(stop_handler,FALSE);
    if(stop_signal){CloseHandle(stop_signal);stop_signal=NULL;}
    if(dll)FreeLibrary(dll);
    if(s)UnmapViewOfFile(s);
    if(mapping)CloseHandle(mapping);
    if(ready)CloseHandle(ready);
    if(host_stop)CloseHandle(host_stop);
    if(security)LocalFree(security);
    return error?error:completed?ERROR_SUCCESS:ERROR_INVALID_STATE;
}
