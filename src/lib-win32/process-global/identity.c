#include "identity.h"
#include "lib-win32/handle/duplicate.h"
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>

DWORD win32_process_global_token_user(HANDLE token,TOKEN_USER **user)
{
    DWORD bytes=0;*user=NULL;
    if(GetTokenInformation(token,TokenUser,NULL,0,&bytes))return ERROR_INVALID_DATA;
    DWORD query_error=GetLastError();
    if(query_error!=ERROR_INSUFFICIENT_BUFFER)return query_error?query_error:ERROR_INVALID_DATA;
    if(bytes<sizeof(TOKEN_USER) || bytes>65536)return ERROR_INVALID_DATA;
    *user=(TOKEN_USER *)malloc(bytes);if(!*user)return ERROR_NOT_ENOUGH_MEMORY;
    if(!GetTokenInformation(token,TokenUser,*user,bytes,&bytes)){DWORD error=GetLastError();free(*user);*user=NULL;return error;}
    return ERROR_SUCCESS;
}

DWORD win32_process_global_desktop_status(DWORD thread_id)
{
    HDESK a=GetThreadDesktop(GetCurrentThreadId());SetLastError(ERROR_SUCCESS);HDESK b=GetThreadDesktop(thread_id);DWORD target_error=GetLastError();
    win32_handle_compare_fn compare=win32_handle_compare_provider();
    /* Names alone do not distinguish desktops in separate window stations. */
    if(!a || !compare)return ERROR_ACCESS_DENIED;
    /* A freshly created thread may not yet be connected to a USER desktop.
     * A null handle without an error is not evidence of a different desktop. */
    if(!b)return target_error?target_error:ERROR_NOT_READY;
    return compare(a,b)?ERROR_SUCCESS:ERROR_ACCESS_DENIED;
}


BOOL win32_process_global_birth(HANDLE process,DWORD *low,DWORD *high)
{
    FILETIME creation,exit,kernel,user;
    if(!process || !low || !high){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!GetProcessTimes(process,&creation,&exit,&kernel,&user))return FALSE;
    *low=creation.dwLowDateTime;*high=creation.dwHighDateTime;return TRUE;
}

BOOL win32_process_global_desktop_key(DWORD *session,ULONGLONG *fingerprint)
{
    WCHAR desktop[256];DWORD needed=0;uint64_t hash=UINT64_C(1469598103934665603);
    if(!session || !fingerprint){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!ProcessIdToSessionId(GetCurrentProcessId(),session))return FALSE;
    HDESK desk=GetThreadDesktop(GetCurrentThreadId());
    if(!desk || !GetUserObjectInformationW(desk,UOI_NAME,desktop,sizeof(desktop),&needed))return FALSE;
    desktop[ARRAYSIZE(desktop)-1]=0;CharUpperBuffW(desktop,(DWORD)wcslen(desktop));
    for(const WCHAR *p=desktop;*p;++p){hash^=(uint16_t)*p;hash*=UINT64_C(1099511628211);}
    *fingerprint=(ULONGLONG)hash;return TRUE;
}
