#include "late_process.h"
#include "lib-win32/process-hook/late_batch.h"
#include "intercept.h"
#include <string.h>
#include <wchar.h>
static SRWLOCK init_lock=SRWLOCK_INIT;
static process_hook_late_patch hooks[88];
static process_hook_late_patch *items[88];
static process_hook_late_batch batch;
static DWORD prepare(void *slot,void *replacement)
{return process_hook_late_batch_prepare(&batch,slot,replacement);}
static DWORD initialize(PCWSTR directory,DWORD session_cookie,DWORD flags,BOOL strict)
{
    if(!(flags&NTW16S_POLICY_PROPAGATE) || (flags&~(NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE)))return ERROR_INVALID_PARAMETER;
    if(!directory)return ERROR_INVALID_PARAMETER;
    size_t n=wcsnlen_s(directory,NTW16S_PATH_CAP);
    BOOL drive=n>=3 && ((directory[0]>=L'A' && directory[0]<=L'Z') || (directory[0]>=L'a' && directory[0]<=L'z')) && directory[1]==L':' && directory[2]==L'\\';
    BOOL unc=n>=5 && directory[0]==L'\\' && directory[1]==L'\\' && directory[2]!=L'\\';
    if(n>=NTW16S_PATH_CAP || (!drive && !unc))return ERROR_INVALID_NAME;
    AcquireSRWLockExclusive(&init_lock);
    DWORD error=ERROR_SUCCESS;
    if(batch.sticky_error){error=batch.sticky_error;goto done;}
    DWORD existing=__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE);
    if(strict && existing && (existing&(NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE))!=flags){error=ERROR_ALREADY_EXISTS;goto done;}
    if(existing && session_cookie){
        /* Adopt an already initialized ordinary process into this session.
         * Publish the cookie first so a concurrent hook cannot observe the
         * session policy before its identity has been installed. */
        __atomic_store_n(&ntw16s_process_context.session_cookie,session_cookie,__ATOMIC_RELEASE);
        __atomic_fetch_or(&ntw16s_process_context.flags,NTW16S_POLICY_GLOBAL_SESSION,__ATOMIC_RELEASE);
        goto done;
    }
    if(batch.installed)goto done;
    if(ntw16s_process_context.flags)goto done;
    memcpy(ntw16s_process_context.directory,directory,(n+1)*sizeof(WCHAR));
    ntw16s_process_context.flags=flags;
    if(session_cookie){ntw16s_process_context.flags|=NTW16S_POLICY_GLOBAL_SESSION;ntw16s_process_context.session_cookie=session_cookie;}
    error=process_hook_late_batch_begin(&batch,hooks,items,ARRAYSIZE(hooks));
    if(!error)error=(DWORD)ntw16s_attach_processes_using(prepare);
    if(!error && (flags&NTW16S_POLICY_STYLE))error=(DWORD)ntw16s_attach_titles_using(prepare);
    if(!error && (flags&NTW16S_POLICY_STYLE))error=(DWORD)ntw16s_attach_file_dialogs_using(prepare);
    if(!error && (flags&NTW16S_POLICY_STYLE))error=(DWORD)ntw16s_attach_com_dialogs_using(prepare);
    if(!error && (flags&NTW16S_POLICY_STYLE))error=(DWORD)ntw16s_attach_folder_dialogs_using(prepare);
    if(!error && batch.count<2)error=ERROR_INVALID_STATE;
    error=process_hook_late_batch_commit(&batch,error,100,1);
    if(error && !batch.sticky_error)ZeroMemory(&ntw16s_process_context,sizeof(ntw16s_process_context));
 done:
    ReleaseSRWLockExclusive(&init_lock);return error;
}

DWORD ntw16s_late_process_init(PCWSTR directory,DWORD session_cookie)
{return initialize(directory,session_cookie,NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE,FALSE);}
DWORD ntw16s_late_process_init_mode(PCWSTR directory,DWORD session_cookie,DWORD flags)
{return initialize(directory,session_cookie,flags,TRUE);}
