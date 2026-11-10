#ifndef WIN16_DIALOG_SHELL_FOLDER_H
#define WIN16_DIALOG_SHELL_FOLDER_H
#include <windows.h>
typedef struct win16_dialog_shell_folder_call {
    HWND owner; PCWSTR title,initial; PWSTR result; DWORD capacity;
} win16_dialog_shell_folder_call;
HRESULT win16_dialog_shell_folder_show(void *data);
#endif
