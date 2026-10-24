#include "elevation.h"
#include <objbase.h>
#include <stdlib.h>
#include <wchar.h>

BOOL run16s_is_elevated(DWORD *error)
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
DWORD run16s_elevate(PCWSTR image,PCWSTR arguments,BOOL wait,
    run16s_shell_execute execute,DWORD *exit_code)
{
    WCHAR self[32768],directory[32768];DWORD n=GetModuleFileNameW(NULL,self,32768);
    if(!n||n>=32768)return n?ERROR_FILENAME_EXCED_RANGE:GetLastError();
    n=GetCurrentDirectoryW(32768,directory);
    if(!n||n>=32768)return n?ERROR_FILENAME_EXCED_RANGE:GetLastError();
    /* Pin the target resolved by the original caller. Only argv[0] is
     * canonicalized; the argument tail is copied verbatim, never reparsed. */
    size_t length=wcslen(image)+wcslen(arguments)+40;
    if(length>32767)return ERROR_FILENAME_EXCED_RANGE;
    WCHAR *parameters=calloc(length,sizeof(WCHAR));
    if(!parameters)return ERROR_NOT_ENOUGH_MEMORY;
    swprintf(parameters,length,L"--elevated %ls\"%ls\"%ls%ls",
        wait?L"--wait ":L"",image,*arguments?L" ":L"",arguments);
    SHELLEXECUTEINFOW info={.cbSize=sizeof(info),
        .fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOASYNC|SEE_MASK_FLAG_NO_UI,
        .lpVerb=L"runas",.lpFile=self,.lpParameters=parameters,
        .lpDirectory=directory,.nShow=SW_SHOWNORMAL};
    HRESULT apartment=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    DWORD error=ERROR_SUCCESS;
    if(FAILED(apartment)&&apartment!=RPC_E_CHANGED_MODE)error=(DWORD)apartment;
    else if(!execute(&info)){error=GetLastError();if(!error)error=ERROR_GEN_FAILURE;}
    else if(!info.hProcess)error=ERROR_INVALID_HANDLE;
    else if(WaitForSingleObject(info.hProcess,INFINITE)!=WAIT_OBJECT_0 ||
            !GetExitCodeProcess(info.hProcess,exit_code))error=GetLastError();
    if(info.hProcess)CloseHandle(info.hProcess);
    if(SUCCEEDED(apartment))CoUninitialize();
    free(parameters);return error;
}
