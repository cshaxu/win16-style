/* Product policy and payload adapter for the shared creation transaction. */
#include "intercept.h"
#include "../../lib-win32/process-hook/startup_hooks.h"
#include "system_process.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32/process-hook/propagate.h"
#include <shellapi.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>
static BOOL (WINAPI *shell_execute_w)(SHELLEXECUTEINFOW *);
static BOOL WINAPI redirect_self_elevation(SHELLEXECUTEINFOW *info)
{
    if(!info || info->cbSize!=sizeof(*info) || info->fMask!=SEE_MASK_FLAG_NO_UI ||
       !info->lpVerb || _wcsicmp(info->lpVerb,L"runas") || !info->lpFile ||
       !ntw16s_process_session_active() ||
       __atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE)!=(NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE))
        return shell_execute_w(info);
    WCHAR *paths=calloc(3u*NTW16S_PATH_CAP,sizeof(*paths));
    if(!paths)return shell_execute_w(info);
    WCHAR *self=paths,*target=paths+NTW16S_PATH_CAP,*launcher=paths+2u*NTW16S_PATH_CAP;
    DWORD n=GetModuleFileNameW(NULL,self,NTW16S_PATH_CAP);
    DWORD m=GetFullPathNameW(info->lpFile,NTW16S_PATH_CAP,target,NULL);
    if(!n || n>=NTW16S_PATH_CAP || !m || m>=NTW16S_PATH_CAP || _wcsicmp(self,target)){
        free(paths);return shell_execute_w(info);
    }
    if(swprintf(launcher,NTW16S_PATH_CAP,L"%lsrun16s.exe",ntw16s_process_context.directory)<0){
        free(paths);return shell_execute_w(info);
    }
    PCWSTR tail=info->lpParameters?info->lpParameters:L"";
    size_t count=wcslen(self)+wcslen(tail)+32;
    if(count>32767){free(paths);return shell_execute_w(info);}
    WCHAR *parameters=calloc(count,sizeof(*parameters));
    if(!parameters){free(paths);return shell_execute_w(info);}
    if(swprintf(parameters,count,L"--elevated \"%ls\"%ls%ls",self,*tail?L" ":L"",tail)<0){free(parameters);free(paths);return shell_execute_w(info);}
    SHELLEXECUTEINFOW request=*info;request.lpFile=launcher;request.lpParameters=parameters;
    BOOL result=shell_execute_w(&request);DWORD error=GetLastError();
    info->hInstApp=request.hInstApp;free(parameters);free(paths);SetLastError(error);return result;
}
BOOL ntw16s_process_session_active(void)
{return !(__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE)&NTW16S_POLICY_GLOBAL_SESSION) || ntw16s_global_policy_active(__atomic_load_n(&ntw16s_process_context.session_cookie,__ATOMIC_ACQUIRE));}
static BOOL active(void *unused){(void)unused;return ntw16s_process_session_active();}
static BOOL accept_w(LPCWSTR application,LPWSTR command,void *unused)
{(void)unused;return !ntw16s_system_print_host_w(application,command);}
static BOOL accept_a(LPCSTR application,LPSTR command,void *unused)
{(void)unused;return !ntw16s_system_print_host_a(application,command);}
static DWORD install(HANDLE child,process_hook_create_w create,void *unused)
{(void)unused;return ntw16s_install(child,&ntw16s_process_context,create);}
LONG ntw16s_attach_processes_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags&NTW16S_POLICY_PROPAGATE))return NO_ERROR;
    const process_hook_propagation_ops ops={active,accept_w,accept_a,install,NULL};
    DWORD error=process_hook_propagation_attach(attach,&ops);
    if(!error)error=process_hook_propagation_attach_as_user_w(attach);
    if(!error && __atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE)==(NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE)){
        HMODULE shell=GetModuleHandleW(L"shell32.dll");
        FARPROC address=shell?GetProcAddress(shell,"ShellExecuteExW"):NULL;
        if(address){shell_execute_w=(void *)address;error=attach(&shell_execute_w,(void *)redirect_self_elevation);}
    }
    return (LONG)error;
}
LONG ntw16s_attach_processes(void){return ntw16s_attach_processes_using(process_hook_startup_attach);}
