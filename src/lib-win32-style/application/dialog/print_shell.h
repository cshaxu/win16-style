#ifndef STYLE_PRINT_SHELL_H
#define STYLE_PRINT_SHELL_H
#include <windows.h>

typedef struct style_print_shell style_print_shell;
style_print_shell *style_print_shell_open(HWND page,HWND placeholder,UINT change,UINT ready);
BOOL style_print_shell_show(style_print_shell *shell,HWND placeholder,PCWSTR initial);
void style_print_shell_close(style_print_shell *shell);
BOOL style_print_shell_select(style_print_shell *shell,PCWSTR name);
/* A successful result is owned by the caller and freed with CoTaskMemFree. */
PWSTR style_print_shell_selected(style_print_shell *shell);

#endif
