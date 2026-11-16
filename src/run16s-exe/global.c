#include "global.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32-style/access/global/owner.h"
#include "lib-win32/process-image/image.h"
#include "lib-win32/security/elevation.h"
#include "ntwshk-dll/injection.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
static DWORD paths(DWORD style,WCHAR *dll64,WCHAR *dll32,DWORD capacity,void *unused)
{
    DWORD error=0;(void)unused;
    ntwshk_context *context=(ntwshk_context *)calloc(1,sizeof(*context));if(!context){error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    error=ntwshk_context_paths(context,NULL);if(error)goto done;
    if(style==NTW16S_GLOBAL_STYLE_2K){
        if(swprintf(dll64,capacity,L"%lsntw2ks64.dll",context->directory)<0 ||
           swprintf(dll32,capacity,L"%lsntw2ks32.dll",context->directory)<0){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
        error=win32_process_image_validate(dll64,IMAGE_FILE_MACHINE_AMD64,TRUE);if(error)goto done;
        error=win32_process_image_validate(dll32,IMAGE_FILE_MACHINE_I386,TRUE);if(error)goto done;
    }
    if(swprintf(dll64,capacity,L"%lsntw16s64.dll",context->directory)<0 || swprintf(dll32,capacity,L"%lsntw16s32.dll",context->directory)<0){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    error=win32_process_image_validate(dll64,IMAGE_FILE_MACHINE_AMD64,TRUE);if(error)goto done;
    error=win32_process_image_validate(dll32,IMAGE_FILE_MACHINE_I386,TRUE);if(error)goto done;
done:
    free(context);return error;
}
static void report(const style_global_owner_notice *notice,void *context)
{
    (void)context;
    switch(notice->kind){
    case STYLE_GLOBAL_OWNER_SWEEP:
        fwprintf(stderr,L"run16s: global %ls: %u windows notified, %u timed out or failed%s\n",
            notice->applying?L"start":L"stop",notice->attempted,notice->failed,notice->deadline?L"; time limit reached":L"");break;
    case STYLE_GLOBAL_OWNER_HOOK_FALLBACK:
        fwprintf(stderr,L"run16s: global x64 hook unavailable (126); using target-thread fallback\n");break;
    case STYLE_GLOBAL_OWNER_HOST_EXIT:fwprintf(stderr,L"run16s: global 32-bit host exited (%lu)\n",notice->error);break;
    case STYLE_GLOBAL_OWNER_PENDING:fwprintf(stderr,L"run16s: global startup attachment still pending\n");break;
    case STYLE_GLOBAL_OWNER_ACTIVE:fwprintf(stderr,L"run16s: global active; press Ctrl+C to stop\n");break;
    case STYLE_GLOBAL_OWNER_PEER_TRACE:fwprintf(stderr,L"run16s: global peer trace cleanup failed (%lu)\n",notice->error);break;
    }
}
static int fail(DWORD error,PCWSTR what)
{fwprintf(stderr,L"run16s: global %ls failed (%lu)\n",what,error);return 10;}
int run16s_global_with_parent(HANDLE parent,DWORD style)
{
    if(style!=NTW16S_GLOBAL_STYLE_16 && style!=NTW16S_GLOBAL_STYLE_2K)return fail(ERROR_NOT_SUPPORTED,L"style");
    if(parent){DWORD state=WaitForSingleObject(parent,0);if(state==WAIT_OBJECT_0)return 0;
        if(state!=WAIT_TIMEOUT)return fail(state==WAIT_FAILED?GetLastError():ERROR_INVALID_HANDLE,L"parent wait");}
    DWORD elevation_error=0;
    if(!win32_security_is_elevated(&elevation_error)){
        if(elevation_error)fwprintf(stderr,L"run16s: unable to check administrator token (%lu)\n",elevation_error);
        else fwprintf(stderr,L"run16s: global background process events require administrator privileges; window discovery remains active\n");
    }
    const style_global_owner_spec spec={&ntw16s_global_manager_config,paths,"Ntw16sGlobalHook",
        L"Ntw16sGlobalHost",report,NULL};
    style_global_manager *retained=NULL;
    DWORD error=style_global_owner_run(&spec,parent,style,&retained);
    return error?fail(error,L"startup or shutdown"):0;
}
