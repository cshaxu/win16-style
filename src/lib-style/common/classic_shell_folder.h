#ifndef STYLE_CLASSIC_SHELL_FOLDER_H
#define STYLE_CLASSIC_SHELL_FOLDER_H
#include <windows.h>
/* Legacy Shell filesystem folder browser; caller owns the output buffer. */
typedef struct style_classic_shell_folder_call {
    HWND owner; PCWSTR title,initial; PWSTR result; DWORD capacity;
} style_classic_shell_folder_call;
HRESULT style_classic_shell_folder_show(void *data);
#endif
