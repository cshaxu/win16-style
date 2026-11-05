#include "identity.h"
#include "lib-win32/handle/duplicate.h"
#include <stdlib.h>

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

