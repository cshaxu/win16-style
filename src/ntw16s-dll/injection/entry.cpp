/* Adapted fixed entry.cpp: passive explicit loads, helper, or copied policy. */
#include "intercept.h"
#include "detours/detours.h"
ntw16s_context ntw16s_process_context={0};
extern "C" void CALLBACK Ntw16sFinishHelper(HWND window,HINSTANCE instance,LPSTR command,INT show)
{DetourFinishHelperProcess(window,instance,command,show);}
extern "C" BOOL ntw16s_bootstrap(void)
{
    if(DetourIsHelperProcess())return TRUE;
    BOOL found=FALSE;
    DWORD error=ntw16s_context_read(&ntw16s_process_context,&found);
    if(error){SetLastError(error);return FALSE;}
    if(!found)return TRUE;
    if(!DetourRestoreAfterWith())return FALSE;
    LONG status=DetourTransactionBegin();
    if(status!=NO_ERROR){SetLastError(status);return FALSE;}
    status=DetourUpdateThread(GetCurrentThread());
    if(!status)status=ntw16s_attach_titles();
    if(status){DetourTransactionAbort();SetLastError(status);return FALSE;}
    status=DetourTransactionCommit();SetLastError(status);return status==NO_ERROR;
}
