#include <windows.h>
#include <wchar.h>
#include "staging.h"
#include "lib-win32-style/access/startup/startup.h"

extern ntwshk_context ntw16s_process_context;
DWORD ntw16s_prepare_startup(void *);
DWORD ntw2ks_prepare_startup(void *);
static ntwshk_context context;

void CALLBACK NtwshkFinishHelper(HWND window,HINSTANCE instance,LPSTR command,INT show)
{
    (void)window;(void)instance;(void)command;(void)show;
    ntwshk_run_helper();
}

static DWORD bootstrap(void)
{
    BOOL found=FALSE;
    DWORD error=style_startup_read(&context,&found);
    /* Explicit loads and loads as a dependency of legacy packages stay passive. */
    if(!found)return ERROR_SUCCESS;
    if(error || _wcsicmp(context.module,L"ntwshk"))return error;
    BOOL win2k=(context.flags&STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE)!=0;
    if(win2k && (context.flags&STYLE_STARTUP_POLICY_GLOBAL_SESSION))return ERROR_INVALID_DATA;
    error=process_hook_stage_restore();
    if(error)return error;
    if(win2k)return win32_style_startup(ntw2ks_prepare_startup,&context);
    ntw16s_process_context=context;
    return win32_style_startup(ntw16s_prepare_startup,NULL);
}

BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID reserved)
{
    (void)instance;(void)reserved;
    if(reason==DLL_PROCESS_ATTACH){
        DWORD error=bootstrap();
        if(error){SetLastError(error);return FALSE;}
    }
    return TRUE;
}
