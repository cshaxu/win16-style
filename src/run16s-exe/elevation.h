#ifndef RUN16S_ELEVATION_H
#define RUN16S_ELEVATION_H
#include <windows.h>
#include <shellapi.h>
typedef BOOL (WINAPI *run16s_shell_execute)(SHELLEXECUTEINFOW *);
/* Only the launcher handles ERROR_ELEVATION_REQUIRED. CreateProcess hooks keep
 * their native failure contract; no shell/broker policy is added to the DLL. */
DWORD run16s_elevate(PCWSTR image,PCWSTR arguments,BOOL wait,BOOL win2k,
    run16s_shell_execute execute,DWORD *exit_code);
BOOL run16s_global_interactive_console(void);
/* Best effort: a missing writable console must not abort the tray process. */
void run16s_rebind_console_stderr(void);
DWORD run16s_elevate_global(DWORD parent_pid,run16s_shell_execute execute,DWORD *exit_code);
#endif
