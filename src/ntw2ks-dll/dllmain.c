#include <windows.h>
#include "ntwshk-dll/injection.h"
#include "lib-win32/process-hook/staging.h"
#include "lib-win32/process-hook/propagate.h"
#include "lib-win32/process-hook/startup_hooks.h"
#include "ntwshk-dll/system_process.h"

void ntwshk_run_helper(void);
DWORD ntw2ks_controls_attach(void);
static ntwshk_context context;
static BOOL active(void *unused){(void)unused;return TRUE;}
static BOOL accept_w(LPCWSTR app,LPWSTR command,void *unused)
{(void)unused;return !ntwshk_system_print_host_w(app,command);}
static BOOL accept_a(LPCSTR app,LPSTR command,void *unused)
{(void)unused;return !ntwshk_system_print_host_a(app,command);}
static DWORD install(HANDLE child,process_hook_create_w create,void *unused)
{(void)unused;return ntwshk_install(child,&context,create);}

static DWORD prepare_hooks(void)
{
    const process_hook_propagation_ops ops={active,accept_w,accept_a,install,NULL};
    DWORD error=process_hook_startup_begin();if(error)return error;
    if(context.flags&NTWSHK_POLICY_PROPAGATE){
        error=process_hook_propagation_attach(process_hook_startup_attach,&ops);
        if(!error)error=process_hook_propagation_attach_as_user_w(process_hook_startup_attach);
    }
    if(!error && (context.flags&NTWSHK_POLICY_STYLE))error=ntw2ks_controls_attach();
    if(error){process_hook_startup_abort();return error;}
    return process_hook_startup_commit();
}

void CALLBACK Ntw2ksFinishHelper(HWND window,HINSTANCE instance,LPSTR command,INT show)
{
    (void)window;(void)instance;(void)command;(void)show;
    ntwshk_run_helper();
}

BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID reserved)
{
    (void)instance;(void)reserved;
    if(reason==DLL_PROCESS_ATTACH){
        BOOL found=FALSE;
        DWORD error=ntwshk_context_read(&context,&found);
        if(!error && found && _wcsicmp(context.module,L"ntw2ks"))error=ERROR_INVALID_DATA;
        if(!error && found && (context.flags&NTWSHK_POLICY_GLOBAL_SESSION))error=ERROR_INVALID_DATA;
        if(!error && found)error=process_hook_stage_restore();
        if(!error && found)error=prepare_hooks();
        if(error){SetLastError(error);return FALSE;}
    }
    return TRUE;
}
