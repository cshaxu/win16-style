#include <windows.h>
#include <stdio.h>
static unsigned scenario;
static BOOL WINAPI simulated_query(HANDLE token,TOKEN_INFORMATION_CLASS type,LPVOID value,DWORD size,PDWORD needed)
{
    (void)token;(void)type;(void)size;
    if(!value){
        *needed=scenario==3?0:sizeof(TOKEN_USER);
        if(scenario==2)return TRUE;
        SetLastError(scenario==0?ERROR_ACCESS_DENIED:scenario==1?ERROR_SUCCESS:ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    if(scenario==4 || scenario==5){SetLastError(scenario==4?ERROR_ACCESS_DENIED:ERROR_SUCCESS);return FALSE;}
    ZeroMemory(value,sizeof(TOKEN_USER));return TRUE;
}
#define GetTokenInformation simulated_query
#include "../../../src/lib-win32/process-global/identity.c"
int main(void)
{
    const DWORD expected[]={ERROR_ACCESS_DENIED,ERROR_INVALID_DATA,ERROR_INVALID_DATA,ERROR_INVALID_DATA,
                            ERROR_ACCESS_DENIED,ERROR_INVALID_DATA,ERROR_SUCCESS};
    for(scenario=0;scenario<ARRAYSIZE(expected);++scenario){
        TOKEN_USER *user=NULL;DWORD error=win32_process_global_token_user((HANDLE)1,&user);
        if(error!=expected[scenario] || (!!user)!=(error==ERROR_SUCCESS))return (int)scenario+1;
        free(user);
    }
    puts("TOKEN_USER_ERRORS_OK query=7 cleanup=1");return 0;
}
