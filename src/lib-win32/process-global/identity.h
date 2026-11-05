#ifndef WIN32_PROCESS_GLOBAL_IDENTITY_H
#define WIN32_PROCESS_GLOBAL_IDENTITY_H
#include <windows.h>
/* Caller owns the returned allocation; the token is borrowed. */
DWORD win32_process_global_token_user(HANDLE token,TOKEN_USER **user);
/* Compares kernel desktop identities for the current and supplied thread. */
DWORD win32_process_global_desktop_status(DWORD thread_id);
#endif
