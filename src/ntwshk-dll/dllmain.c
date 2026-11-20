#include <windows.h>
#include <wchar.h>
#include "staging.h"
#include "lib-win32-style/access/startup/startup.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "ntw16s-dll/global/session.h"

extern ntwshk_context ntw16s_process_context;
DWORD ntw16s_prepare_startup(void *);
DWORD ntw2ks_prepare_startup(void *);
static ntwshk_context context;
static BOOL global_profile(unsigned *profile)
{
    DWORD selected;
    if(!(__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE)&NTWSHK_POLICY_GLOBAL_SESSION) ||
       !style_global_policy_current(&ntw16s_global_namespace,
           __atomic_load_n(&ntw16s_process_context.session_cookie,__ATOMIC_ACQUIRE),&selected))return FALSE;
    *profile=selected;return TRUE;
}

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
    unsigned profile=context.flags>>STYLE_STARTUP_PROFILE_SHIFT;
    classic_theme_family family=win2k?CLASSIC_THEME_WIN2K:CLASSIC_THEME_WIN16;
    classic_theme_set_process_profile(profile?profile:(unsigned)family,family);
    if(win2k)return win32_style_startup(ntw2ks_prepare_startup,&context);
    ntw16s_process_context=context;
    return win32_style_startup(ntw16s_prepare_startup,NULL);
}

BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID reserved)
{
    (void)instance;(void)reserved;
    if(reason==DLL_PROCESS_ATTACH){
        classic_theme_set_global_profile_reader(global_profile);
        DWORD error=bootstrap();
        if(error){SetLastError(error);return FALSE;}
    }
    return TRUE;
}
