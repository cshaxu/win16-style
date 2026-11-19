#ifndef WIN2K_PRINT_SHELL_H
#define WIN2K_PRINT_SHELL_H
#include <windows.h>

typedef struct win2k_print_shell win2k_print_shell;
win2k_print_shell *win2k_print_shell_open(HWND page,HWND placeholder,UINT change,UINT ready);
BOOL win2k_print_shell_show(win2k_print_shell *shell,HWND placeholder,PCWSTR initial);
void win2k_print_shell_close(win2k_print_shell *shell);
BOOL win2k_print_shell_select(win2k_print_shell *shell,PCWSTR name);
/* A successful result is owned by the caller and freed with CoTaskMemFree. */
PWSTR win2k_print_shell_selected(win2k_print_shell *shell);

#endif
