#include "elevation.h"
#include <objbase.h>
BOOL win32_security_is_elevated(DWORD *error)
{
    HANDLE token=NULL;TOKEN_ELEVATION elevation={0};DWORD bytes=0;
    *error=ERROR_SUCCESS;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)){
        *error=GetLastError();return FALSE;
    }
    BOOL ok=GetTokenInformation(token,TokenElevation,&elevation,sizeof(elevation),&bytes);
    if(!ok)*error=GetLastError();
    CloseHandle(token);return ok && elevation.TokenIsElevated;
}
DWORD win32_security_shell_wait(SHELLEXECUTEINFOW *info,
    BOOL (WINAPI *execute)(SHELLEXECUTEINFOW *),BOOL ignore_console_ctrl,DWORD *exit_code)
{
    DWORD error=ERROR_SUCCESS;BOOL handler=FALSE;
    if(!info || !execute || !exit_code || info->hProcess)return ERROR_INVALID_PARAMETER;
    HRESULT apartment=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(apartment) && apartment!=RPC_E_CHANGED_MODE)error=(DWORD)apartment;
    else if(ignore_console_ctrl && !(handler=SetConsoleCtrlHandler(NULL,TRUE)))error=GetLastError();
    else if(!execute(info)){error=GetLastError();if(!error)error=ERROR_GEN_FAILURE;}
    else if(!info->hProcess)error=ERROR_INVALID_HANDLE;
    else if(WaitForSingleObject(info->hProcess,INFINITE)!=WAIT_OBJECT_0 ||
            !GetExitCodeProcess(info->hProcess,exit_code)){
        error=GetLastError();if(!error)error=ERROR_GEN_FAILURE;
    }
    if(handler)SetConsoleCtrlHandler(NULL,FALSE);
    if(info->hProcess){CloseHandle(info->hProcess);info->hProcess=NULL;}
    if(SUCCEEDED(apartment))CoUninitialize();
    return error;
}
