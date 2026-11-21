#include <windows.h>
#include <wchar.h>
#include "staging.h"
#include "observed.h"
#include "lib-win32-style/access/startup/startup.h"
#include "lib-win32-style/application/control/updown.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "ntw16s-dll/global/session.h"

#include "ntw16s-dll/injection/intercept.h"
#include "ntw16s-dll/api.h"
ntwshk_context ntw16s_process_context;
static ntwshk_context context;

DWORD __cdecl Ntw16sGetAbiVersion(void)
{return NTW16S_ABI_VERSION;}

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

static DWORD prepare_observed(void)
{
    DWORD (*attach)(void *,void *)=process_hook_startup_attach;
    DWORD error=(DWORD)ntw16s_attach_startup_windows_using(attach,ntwshk_observed_window_created);
    if(!error)error=(DWORD)ntw16s_attach_startup_dialogs_using(attach);
    if(!error)error=(DWORD)ntw16s_attach_startup_print_using(attach);
    if(!error)error=(DWORD)ntw16s_attach_startup_stock_using(attach);
    return error;
}

static DWORD prepare_startup(void *value)
{
    const enum classic_window_provider layout=*(const enum classic_window_provider *)value;
    const BOOL observed=layout==CLASSIC_WINDOW_PROVIDER_NONCLIENT;
    DWORD error=(DWORD)ntw16s_attach_startup_processes(observed?&context:NULL);
    if(error || !(context.flags&NTWSHK_POLICY_STYLE))return error;
    error=observed?prepare_observed():
        (DWORD)ntw16s_attach_titles_using(process_hook_startup_attach);
    return error?error:(DWORD)ntw16s_attach_shell_dialogs_using(process_hook_startup_attach,observed);
}

static DWORD bootstrap(void)
{
    BOOL found=FALSE;
    DWORD error=style_startup_read(&context,&found);
    /* Explicit loads and loads as a dependency of legacy packages stay passive. */
    if(!found)return ERROR_SUCCESS;
    if(error || _wcsicmp(context.module,L"ntwshk"))return error;
    const style_startup_selection wire=style_startup_get_selection(context.flags);
    const classic_theme_selection selection={wire.theme_id,(classic_theme_family)wire.adapter};
    enum classic_window_provider layout=classic_theme_selection_layout(selection);
    const BOOL global=(context.flags&STYLE_STARTUP_POLICY_GLOBAL_SESSION)!=0;
    const BOOL styled=(context.flags&NTWSHK_POLICY_STYLE)!=0;
    if(layout==CLASSIC_WINDOW_PROVIDER_UNSPECIFIED ||
       (global && layout!=CLASSIC_WINDOW_PROVIDER_FULL_CLIENT) ||
       (styled && !global && !classic_theme_is_folder_id(selection.id)))return ERROR_INVALID_DATA;
    error=process_hook_stage_restore();
    if(error)return error;
    /* A global child gets appearance only from its active session. */
    if(styled && !global)classic_theme_set_process_profile(selection.id,selection.family);
    if(layout==CLASSIC_WINDOW_PROVIDER_FULL_CLIENT)ntw16s_process_context=context;
    error=win32_style_startup(prepare_startup,&layout);
    if(!error && styled)style_control_updown_api_enable();
    return error;
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
