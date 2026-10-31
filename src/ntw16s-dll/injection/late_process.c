#include "late_process.h"
#include "late_patch.h"
#include "intercept.h"
#include "minhook/src/buffer.h"
#include <string.h>
#include <wchar.h>
static SRWLOCK init_lock=SRWLOCK_INIT;
static ntw16s_late_patch hooks[2];
static ntw16s_late_patch *items[2];
static ntw16s_late_guard guard;
static DWORD count,sticky_error;
static BOOL installed;
static DWORD prepare(void *slot,void *replacement)
{
    if(count>=ARRAYSIZE(hooks))return ERROR_INSUFFICIENT_BUFFER;
    DWORD error=ntw16s_late_patch_prepare(&hooks[count],slot,replacement);
    if(!error){items[count]=&hooks[count];++count;}return error;
}
DWORD ntw16s_late_process_init(PCWSTR directory,DWORD session_cookie)
{
    if(!directory)return ERROR_INVALID_PARAMETER;
    size_t n=wcsnlen_s(directory,NTW16S_PATH_CAP);
    BOOL drive=n>=3 && ((directory[0]>=L'A' && directory[0]<=L'Z') || (directory[0]>=L'a' && directory[0]<=L'z')) && directory[1]==L':' && directory[2]==L'\\';
    BOOL unc=n>=5 && directory[0]==L'\\' && directory[1]==L'\\' && directory[2]!=L'\\';
    if(n>=NTW16S_PATH_CAP || (!drive && !unc))return ERROR_INVALID_NAME;
    AcquireSRWLockExclusive(&init_lock);
    DWORD error=ERROR_SUCCESS;
    if(sticky_error){error=sticky_error;goto done;}
    if((ntw16s_process_context.flags&NTW16S_POLICY_GLOBAL_SESSION) && session_cookie){
        InterlockedExchange((volatile LONG *)&ntw16s_process_context.session_cookie,(LONG)session_cookie);goto done;
    }
    if(installed)goto done;
    if(ntw16s_process_context.flags)goto done;
    memcpy(ntw16s_process_context.directory,directory,(n+1)*sizeof(WCHAR));
    ntw16s_process_context.flags=NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE;
    if(session_cookie){ntw16s_process_context.flags|=NTW16S_POLICY_GLOBAL_SESSION;ntw16s_process_context.session_cookie=session_cookie;}
    InitializeBuffer();error=(DWORD)ntw16s_attach_processes_using(prepare);
    if(!error && count!=ARRAYSIZE(hooks))error=ERROR_INVALID_STATE;
    if(!error){
        error=ERROR_BUSY;
        for(unsigned i=0;i<100 && (error==ERROR_BUSY || error==ERROR_RETRY);++i){
            error=ntw16s_late_patch_install_batch(&guard,items,count);
            if(error==ERROR_BUSY || error==ERROR_RETRY)Sleep(1);
        }
    }
    if(!error){installed=TRUE;goto done;}
    if(guard.threads || hooks[0].installed || hooks[1].installed){sticky_error=error;goto done;}
    for(DWORD i=0;i<count;++i)(void)ntw16s_late_patch_discard(&hooks[i]);
    count=0;ZeroMemory(&ntw16s_process_context,sizeof(ntw16s_process_context));
 done:
    ReleaseSRWLockExclusive(&init_lock);return error;
}
