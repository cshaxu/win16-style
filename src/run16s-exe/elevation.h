#ifndef RUN16S_ELEVATION_H
#define RUN16S_ELEVATION_H
#include <windows.h>
#include <shellapi.h>
typedef BOOL (WINAPI *run16s_shell_execute)(SHELLEXECUTEINFOW *);
BOOL run16s_is_elevated(DWORD *error);
/* Only the launcher handles ERROR_ELEVATION_REQUIRED. CreateProcess hooks keep
 * their native failure contract; no shell/broker policy is added to the DLL. */
DWORD run16s_elevate(PCWSTR image,PCWSTR arguments,BOOL wait,
    run16s_shell_execute execute,DWORD *exit_code);
#endif
