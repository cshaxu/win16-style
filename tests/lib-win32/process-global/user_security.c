#include "lib-win32/process-global/identity.h"
#include <sddl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
int main(void)
{
    HANDLE token=NULL;TOKEN_USER *user=NULL;LPWSTR sid=NULL;DWORD error;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return 1;
    error=win32_process_global_token_user(token,&user);CloseHandle(token);
    if(error || !user)return 2;
    if(!ConvertSidToStringSidW(user->User.Sid,&sid)){free(user);return 3;}
    free(user);
    for(unsigned low=0;low<2;++low){
        WCHAR text[512];PSECURITY_DESCRIPTOR actual=NULL,expected=NULL;
        if(swprintf(text,ARRAYSIZE(text),low?L"D:P(A;;GA;;;SY)(A;;GA;;;%ls)S:(ML;;NW;;;LW)":L"D:P(A;;GA;;;SY)(A;;GA;;;%ls)",sid)<0){LocalFree(sid);return 4;}
        if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(text,SDDL_REVISION_1,&expected,NULL)){LocalFree(sid);return 5;}
        error=win32_process_global_user_security((BOOL)low,&actual);
        DWORD bytes=GetSecurityDescriptorLength(expected);
        BOOL same=!error && actual && GetSecurityDescriptorLength(actual)==bytes && !memcmp(actual,expected,bytes);
        if(actual)LocalFree(actual);
        LocalFree(expected);
        if(!same){LocalFree(sid);return (int)low+6;}
    }
    LocalFree(sid);puts("USER_SECURITY_OK session=1 low_event=1");return 0;
}
