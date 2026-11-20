#include "global.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32-style/access/global/owner.h"
#include "lib-win32-style/access/startup/context.h"
#include "lib-win32/process-image/image.h"
#include "lib-win32/security/elevation.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <stdio.h>
static DWORD paths(DWORD style,WCHAR *dll64,WCHAR *dll32,DWORD capacity,void *unused)
{
    (void)style;(void)unused;
    style_startup_context context;
    DWORD error=style_startup_paths(&context,NULL);if(error)return error;
    error=style_startup_module_path(&context,IMAGE_FILE_MACHINE_AMD64,L"ntwshk",dll64,capacity);if(error)return error;
    error=style_startup_module_path(&context,IMAGE_FILE_MACHINE_I386,L"ntwshk",dll32,capacity);if(error)return error;
    error=win32_process_image_validate(dll64,IMAGE_FILE_MACHINE_AMD64,TRUE);if(error)return error;
    return win32_process_image_validate(dll32,IMAGE_FILE_MACHINE_I386,TRUE);
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
    classic_theme_selection selection;
    if(!classic_theme_decode_selection(style,&selection))
        return fail(ERROR_NOT_SUPPORTED,L"style");
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
