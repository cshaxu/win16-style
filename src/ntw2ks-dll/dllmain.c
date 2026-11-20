#include <windows.h>
#include "ntwshk-dll/injection.h"
#include "lib-win32/process-hook/staging.h"
#include "lib-win32/process-hook/propagate.h"
#include "lib-win32/process-hook/startup_hooks.h"
#include "lib-win32-style/access/startup/propagate.h"
#include "lib-win32-style/access/startup/startup.h"

void ntwshk_run_helper(void);
DWORD ntw2ks_controls_attach(void);
static style_startup_propagation propagation;
DWORD ntw2ks_prepare_startup(void *value)
{
    const ntwshk_context *context=value;
    propagation=(style_startup_propagation){context,L"ntwshk",NULL,NULL};
    DWORD error=style_startup_propagate(process_hook_startup_attach,&propagation);
    if(!error && (context->flags&NTWSHK_POLICY_STYLE))error=ntw2ks_controls_attach();
    return error;
}

#ifndef NTWSHK_UNIFIED
static ntwshk_context context;

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
        DWORD error=style_startup_bootstrap(&context,L"ntw2ks",FALSE,&found,ntw2ks_prepare_startup,&context);
        if(error){SetLastError(error);return FALSE;}
    }
    return TRUE;
}
#endif
