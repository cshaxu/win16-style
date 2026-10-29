/* Adapted fixed entry.cpp: passive explicit loads, helper, or copied policy. */
#include "intercept.h"
#include "startup_hooks.h"
#include "staging.h"
ntw16s_context ntw16s_process_context={0};
void CALLBACK Ntw16sFinishHelper(HWND window,HINSTANCE instance,LPSTR command,INT show)
{(void)window;(void)instance;(void)command;(void)show;ntw16s_run_helper();}
BOOL ntw16s_bootstrap(void)
{
    BOOL found=FALSE;
    DWORD error=ntw16s_context_read(&ntw16s_process_context,&found);
    if(error){SetLastError(error);return FALSE;}
    if(!found)return TRUE;
    error=ntw16s_stage_restore();if(error){SetLastError(error);return FALSE;}
    LONG status=(LONG)ntw16s_hooks_begin();
    if(status!=NO_ERROR){SetLastError(status);return FALSE;}
    status=ntw16s_attach_processes();
    if(!status)status=ntw16s_attach_titles();
    if(!status)status=ntw16s_attach_file_dialogs();
    if(!status)status=ntw16s_attach_com_dialogs();
    if(!status)status=ntw16s_attach_folder_dialogs();
    if(status){ntw16s_hooks_abort();SetLastError(status);return FALSE;}
    status=(LONG)ntw16s_hooks_commit();SetLastError(status);return status==NO_ERROR;
}
