/* Fixed-source native creation transaction, without broker/VDM redirection. */
#include "intercept.h"
#include "detours/detours.h"
static decltype(&CreateProcessW) create_w=CreateProcessW;
static decltype(&CreateProcessA) create_a=CreateProcessA;
static __declspec(thread) unsigned entering;
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
static BOOL WINAPI hook_w(LPCWSTR application,LPWSTR command,LPSECURITY_ATTRIBUTES pa,
    LPSECURITY_ATTRIBUTES ta,BOOL inherit,DWORD flags,LPVOID environment,LPCWSTR directory,
    LPSTARTUPINFOW startup,LPPROCESS_INFORMATION child)
{
    if(entering || !startup || !child)return create_w(application,command,pa,ta,inherit,flags,environment,directory,startup,child);
    BOOL result=FALSE;DWORD error=ERROR_SUCCESS;++entering;
    __try {
        result=create_w(application,command,pa,ta,inherit,flags|CREATE_SUSPENDED,environment,directory,startup,child);
        error=GetLastError();result=finish(result,flags,child,error);error=GetLastError();
    } __finally {--entering;}
    SetLastError(error);return result;
}
static BOOL WINAPI hook_a(LPCSTR application,LPSTR command,LPSECURITY_ATTRIBUTES pa,
    LPSECURITY_ATTRIBUTES ta,BOOL inherit,DWORD flags,LPVOID environment,LPCSTR directory,
    LPSTARTUPINFOA startup,LPPROCESS_INFORMATION child)
{
    if(entering || !startup || !child)return create_a(application,command,pa,ta,inherit,flags,environment,directory,startup,child);
    BOOL result=FALSE;DWORD error=ERROR_SUCCESS;++entering;
    __try {
        result=create_a(application,command,pa,ta,inherit,flags|CREATE_SUSPENDED,environment,directory,startup,child);
        error=GetLastError();result=finish(result,flags,child,error);error=GetLastError();
    } __finally {--entering;}
    SetLastError(error);return result;
}
LONG ntw16s_attach_processes(void)
{
    if(!(ntw16s_process_context.flags&NTW16S_POLICY_PROPAGATE))return NO_ERROR;
    LONG error=DetourAttach(&(PVOID &)create_w,hook_w);
    if(!error)error=DetourAttach(&(PVOID &)create_a,hook_a);
    return error;
}
