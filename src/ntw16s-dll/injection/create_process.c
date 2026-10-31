/* Fixed-source native creation transaction, without broker/VDM redirection. */
#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
#include "system_process.h"
static __typeof__(&CreateProcessW) create_w;
static __typeof__(&CreateProcessA) create_a;
static _Thread_local unsigned entering;
static void leave_process(void *unused){(void)unused;--entering;}
static BOOL finish(BOOL created,DWORD flags,PROCESS_INFORMATION *child,DWORD original_error)
{
    if(!created){SetLastError(original_error);return FALSE;}
    DWORD error=ntw16s_install(child->hProcess,&ntw16s_process_context,create_w);
    if(!error && !(flags&CREATE_SUSPENDED)){
        DWORD previous=ResumeThread(child->hThread);
        if(previous!=1)error=previous==(DWORD)-1?GetLastError():ERROR_INVALID_STATE;
    }
    if(error){
        /* Only this layer's newly created, uncommitted process is owned. */
        TerminateProcess(child->hProcess,error);WaitForSingleObject(child->hProcess,5000);
        CloseHandle(child->hThread);CloseHandle(child->hProcess);ZeroMemory(child,sizeof(*child));
        SetLastError(error);return FALSE;
    }
    SetLastError(original_error);return TRUE;
}
/* Spell out A/W prototypes: native encoding and every original field remain
 * untouched. The local guard prevents native A->W duplicate interception.
 * Helpers receive the captured W trampoline, not the patched export. */
static NTW16S_UNWIND_FUNCTION BOOL WINAPI hook_w(LPCWSTR application,LPWSTR command,LPSECURITY_ATTRIBUTES pa,
    LPSECURITY_ATTRIBUTES ta,BOOL inherit,DWORD flags,LPVOID environment,LPCWSTR directory,
    LPSTARTUPINFOW startup,LPPROCESS_INFORMATION child)
{
    if(entering || !startup || !child || ntw16s_system_print_host_w(application,command))return create_w(application,command,pa,ta,inherit,flags,environment,directory,startup,child);
    BOOL result=FALSE;DWORD error=ERROR_SUCCESS;ntw16s_unwind_scope scope;++entering;
    NTW16S_UNWIND_BEGIN(scope,leave_process,NULL);
    {
        result=create_w(application,command,pa,ta,inherit,flags|CREATE_SUSPENDED,environment,directory,startup,child);
        error=GetLastError();result=finish(result,flags,child,error);error=GetLastError();
    }
    NTW16S_UNWIND_END(scope);
    SetLastError(error);return result;
}
static NTW16S_UNWIND_FUNCTION BOOL WINAPI hook_a(LPCSTR application,LPSTR command,LPSECURITY_ATTRIBUTES pa,
    LPSECURITY_ATTRIBUTES ta,BOOL inherit,DWORD flags,LPVOID environment,LPCSTR directory,
    LPSTARTUPINFOA startup,LPPROCESS_INFORMATION child)
{
    if(entering || !startup || !child || ntw16s_system_print_host_a(application,command))return create_a(application,command,pa,ta,inherit,flags,environment,directory,startup,child);
    BOOL result=FALSE;DWORD error=ERROR_SUCCESS;ntw16s_unwind_scope scope;++entering;
    NTW16S_UNWIND_BEGIN(scope,leave_process,NULL);
    {
        result=create_a(application,command,pa,ta,inherit,flags|CREATE_SUSPENDED,environment,directory,startup,child);
        error=GetLastError();result=finish(result,flags,child,error);error=GetLastError();
    }
    NTW16S_UNWIND_END(scope);
    SetLastError(error);return result;
}
LONG ntw16s_attach_processes_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags&NTW16S_POLICY_PROPAGATE))return NO_ERROR;
    create_w=CreateProcessW;create_a=CreateProcessA;
    LONG error=(LONG)attach(&create_w,(void *)hook_w);
    if(!error)error=(LONG)attach(&create_a,(void *)hook_a);
    return error;
}
LONG ntw16s_attach_processes(void){return ntw16s_attach_processes_using(ntw16s_hooks_attach);}
