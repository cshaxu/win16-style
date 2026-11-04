#ifndef WIN32_SECURITY_ELEVATION_H
#define WIN32_SECURITY_ELEVATION_H
#include <windows.h>
#include <shellapi.h>
#ifdef __cplusplus
extern "C" {
#endif
/* error is required and receives the original Win32 error, or zero. */
BOOL win32_security_is_elevated(DWORD *error);
/* info and callback are borrowed; info->hProcess must initially be NULL. Any
 * returned process handle is waited, closed and cleared. The caller chooses
 * all Shell parameters and presentation policy. */
DWORD win32_security_shell_wait(SHELLEXECUTEINFOW *info,
    BOOL (WINAPI *execute)(SHELLEXECUTEINFOW *),BOOL ignore_console_ctrl,DWORD *exit_code);
#ifdef __cplusplus
}
#endif
#endif
