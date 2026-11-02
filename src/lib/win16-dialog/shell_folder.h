#ifndef WIN16_DIALOG_SHELL_FOLDER_H
#define WIN16_DIALOG_SHELL_FOLDER_H
#include <windows.h>
typedef struct win16_dialog_shell_folder_call {
    HWND owner; PCWSTR title,initial,ok_label,cancel_label,file_label; PWSTR result; DWORD capacity;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    void *proxy;
#endif
} win16_dialog_shell_folder_call;
HRESULT win16_dialog_shell_folder_show(void *data);
#endif
