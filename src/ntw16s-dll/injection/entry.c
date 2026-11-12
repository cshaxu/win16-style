/* Adapted fixed entry.cpp: passive explicit loads, helper, or copied policy. */
#include "intercept.h"
#include "../../lib-win32/process-hook/startup_hooks.h"
#include "ntwshk-dll/staging.h"
#include "lib-win32/style/startup.h"
ntwshk_context ntw16s_process_context={0};
void CALLBACK Ntw16sFinishHelper(HWND window,HINSTANCE instance,LPSTR command,INT show)
{(void)window;(void)instance;(void)command;(void)show;ntwshk_run_helper();}
static DWORD prepare_hooks(void *unused)
{
    (void)unused;LONG status=ntw16s_attach_processes();
    if(!status)status=ntw16s_attach_titles();
    if(!status)status=ntw16s_attach_file_dialogs();
    if(!status)status=ntw16s_attach_com_dialogs();
    if(!status)status=ntw16s_attach_folder_dialogs();
    return (DWORD)status;
}
BOOL ntw16s_bootstrap(void)
{
    BOOL found=FALSE;
    DWORD error=ntwshk_context_read(&ntw16s_process_context,&found);
    if(error){SetLastError(error);return FALSE;}
    if(!found)return TRUE;
    if(_wcsicmp(ntw16s_process_context.module,L"ntw16s")){SetLastError(ERROR_INVALID_DATA);return FALSE;}
    error=process_hook_stage_restore();if(error){SetLastError(error);return FALSE;}
    error=win32_style_startup(prepare_hooks,NULL);SetLastError(error);return error==NO_ERROR;
}
