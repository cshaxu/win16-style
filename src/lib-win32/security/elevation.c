#include "elevation.h"
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
