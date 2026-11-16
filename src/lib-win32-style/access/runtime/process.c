#include "process.h"
#include "lib-win32/process-hook/propagate.h"
#include <string.h>
#include <wchar.h>

/* The backend's attach signature has no context parameter. The binding is
 * borrowed only by synchronous registration and restored before commit. */
static _Thread_local process_hook_late_batch *preparing;
static DWORD attach(void *slot,void *replacement)
{return process_hook_late_batch_prepare(preparing,slot,replacement);}
DWORD style_runtime_process_init(style_runtime_process *state,style_startup_context *process,
    const style_runtime_process_ops *ops,PCWSTR directory,DWORD cookie,DWORD flags,BOOL strict)
{
    if(!(flags&STYLE_STARTUP_POLICY_PROPAGATE) ||
       (flags&~(STYLE_STARTUP_POLICY_STYLE|STYLE_STARTUP_POLICY_PROPAGATE)))return ERROR_INVALID_PARAMETER;
    if(!directory || !ops->module || wcsnlen_s(ops->module,STYLE_STARTUP_MODULE_CAP)>=STYLE_STARTUP_MODULE_CAP)
        return ERROR_INVALID_PARAMETER;
    size_t n=wcsnlen_s(directory,STYLE_STARTUP_PATH_CAP);
    BOOL drive=n>=3 && ((directory[0]>=L'A' && directory[0]<=L'Z') || (directory[0]>=L'a' && directory[0]<=L'z')) && directory[1]==L':' && directory[2]==L'\\';
    BOOL unc=n>=5 && directory[0]==L'\\' && directory[1]==L'\\' && directory[2]!=L'\\';
    if(n>=STYLE_STARTUP_PATH_CAP || (!drive && !unc))return ERROR_INVALID_NAME;
    AcquireSRWLockExclusive(&state->lock);
    DWORD error=ERROR_SUCCESS;
    if(state->batch.sticky_error){error=state->batch.sticky_error;goto done;}
    DWORD existing=__atomic_load_n(&process->flags,__ATOMIC_ACQUIRE);
    if(strict && existing && (existing&(STYLE_STARTUP_POLICY_STYLE|STYLE_STARTUP_POLICY_PROPAGATE))!=flags){error=ERROR_ALREADY_EXISTS;goto done;}
    if(existing && cookie){
        __atomic_store_n(&process->session_cookie,cookie,__ATOMIC_RELEASE);
        __atomic_fetch_or(&process->flags,STYLE_STARTUP_POLICY_GLOBAL_SESSION,__ATOMIC_RELEASE);
        goto done;
    }
    if(state->batch.installed || process->flags)goto done;
    BOOL reuse=process_hook_propagation_registered();
    if(reuse && !cookie){error=ERROR_ALREADY_EXISTS;goto done;}
    memcpy(process->directory,directory,(n+1)*sizeof(WCHAR));
    wcscpy(process->module,ops->module);process->flags=flags;
    if(cookie){process->flags|=STYLE_STARTUP_POLICY_GLOBAL_SESSION;process->session_cookie=cookie;}
    error=process_hook_late_batch_begin(&state->batch,state->hooks,state->items,ARRAYSIZE(state->hooks));
    process_hook_late_batch *previous=preparing;preparing=&state->batch;
    if(!error)error=ops->prepare(attach,reuse,flags,ops->context);
    preparing=previous;
    if(!error && !reuse && state->batch.count<2)error=ERROR_INVALID_STATE;
    if(!error && reuse && !state->batch.count)state->batch.installed=TRUE;
    else error=process_hook_late_batch_commit(&state->batch,error,100,1);
    if(error && !state->batch.sticky_error){
        if(!reuse)ops->discard(ops->context);
        ZeroMemory(process,sizeof(*process));
    }
 done:
    ReleaseSRWLockExclusive(&state->lock);return error;
}
