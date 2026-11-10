#ifndef WIN32_PROCESS_GLOBAL_IDENTITY_H
#define WIN32_PROCESS_GLOBAL_IDENTITY_H
#include <windows.h>
/* Caller owns the returned allocation; the token is borrowed. */
DWORD win32_process_global_token_user(HANDLE token,TOKEN_USER **user);
/* Caller frees the returned descriptor with LocalFree. */
DWORD win32_process_global_user_security(BOOL low_integrity,PSECURITY_DESCRIPTOR *descriptor);
/* Compares kernel desktop identities for the current and supplied thread. */
DWORD win32_process_global_desktop_status(DWORD thread_id);
/* Current session and case-insensitive desktop-name fingerprint. */
BOOL win32_process_global_desktop_key(DWORD *session,ULONGLONG *fingerprint);
/* Borrowed process handle; returns its immutable creation timestamp. */
BOOL win32_process_global_birth(HANDLE process,DWORD *low,DWORD *high);
#endif
