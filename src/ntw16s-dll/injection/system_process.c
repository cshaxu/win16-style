#include "system_process.h"
#include <wchar.h>
/* The Windows print subsystem creates this shared host on demand. It is a
 * system service process, not an application descendant to be styled. */
BOOL ntw16s_system_print_host_w(LPCWSTR application,LPCWSTR command)
{
    WCHAR system[MAX_PATH],expected[MAX_PATH],candidate[MAX_PATH];UINT length=GetSystemWindowsDirectoryW(system,MAX_PATH);
    if(!length || length>=MAX_PATH)return FALSE;
    if(swprintf(expected,MAX_PATH,L"%ls%lssplwow64.exe",system,system[length-1]==L'\\'?L"":L"\\")<0)return FALSE;
    if(application && *application)return !_wcsicmp(application,expected);
    if(!command)return FALSE;
    while(*command==L' ' || *command==L'\t')++command;
    BOOL quoted=*command==L'"';if(quoted)++command;
    const WCHAR *end=command;while(*end && (quoted?*end!=L'"':(*end!=L' ' && *end!=L'\t')))++end;
    size_t count=(size_t)(end-command);if(!count || count>=MAX_PATH || (quoted && *end!=L'"'))return FALSE;
    wmemcpy(candidate,command,count);candidate[count]=0;return !_wcsicmp(candidate,expected);
}
BOOL ntw16s_system_print_host_a(LPCSTR application,LPCSTR command)
{
    WCHAR app[MAX_PATH],cmd[2048];LPCWSTR wide_app=NULL,wide_cmd=NULL;
    if(application){if(!MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,application,-1,app,MAX_PATH))return FALSE;wide_app=app;}
    if(command && !application){if(!MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,command,-1,cmd,2048))return FALSE;wide_cmd=cmd;}
    return ntw16s_system_print_host_w(wide_app,wide_cmd);
}

