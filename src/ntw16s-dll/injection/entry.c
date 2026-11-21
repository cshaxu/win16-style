/* Adapted fixed entry.cpp: passive explicit loads, helper, or copied policy. */
#include "intercept.h"
#include "../../lib-win32/process-hook/startup_hooks.h"
#include "ntwshk-dll/staging.h"
#include "lib-win32-style/access/startup/startup.h"
ntwshk_context ntw16s_process_context={0};
void CALLBACK Ntw16sFinishHelper(HWND window,HINSTANCE instance,LPSTR command,INT show)
{(void)window;(void)instance;(void)command;(void)show;ntwshk_run_helper();}
static DWORD prepare_startup(void *unused)
{
    (void)unused;LONG status=ntw16s_attach_processes_using(process_hook_startup_attach);
    if(!status)status=ntw16s_attach_titles_using(process_hook_startup_attach);
    if(!status)status=ntw16s_attach_shell_dialogs_using(process_hook_startup_attach,FALSE);
    return (DWORD)status;
}
BOOL ntw16s_bootstrap(void)
{
    BOOL found=FALSE;
    DWORD error=style_startup_bootstrap(&ntw16s_process_context,L"ntw16s",TRUE,&found,prepare_startup,NULL);
    if(error || found)SetLastError(error);
    return error==NO_ERROR;
}
