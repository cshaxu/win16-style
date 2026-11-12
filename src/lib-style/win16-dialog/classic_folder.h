#ifndef WIN16_DIALOG_CLASSIC_FOLDER_H
#define WIN16_DIALOG_CLASSIC_FOLDER_H
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
typedef UINT_PTR (*win16_dialog_folder_observer)(HWND,UINT,WPARAM,LPARAM,void *);
typedef struct win16_dialog_classic_folder_context {
    BFFCALLBACK callback;
    PCWSTR ok_label,cancel_label,file_label;
    LPARAM original_data;
    WCHAR selected[MAX_PATH],desired[MAX_PATH],requested[MAX_PATH],classic_current[MAX_PATH];
    UINT run_message,attempts;
    DWORD init_tick,changed_tick;
    BOOL showing;
    HWND callback_window;
    BOOL experimental;
    win16_dialog_folder_observer observer;void *observer_context;
    HWND classic_window;
} win16_dialog_classic_folder_context;
HRESULT win16_dialog_classic_folder_prepare(win16_dialog_classic_folder_context *,OPENFILENAMEW *,WCHAR *,
    HWND,PCWSTR,PCWSTR,PCWSTR,PCWSTR,PCWSTR,LPOFNHOOKPROC);
HRESULT win16_dialog_classic_folder_finish(BOOL,DWORD,WCHAR *,PWSTR,DWORD);
/* WM_INITDIALOG receives the OPENFILENAMEW prepared above, with its live context. */
UINT_PTR win16_dialog_classic_folder_hook(HWND,UINT,WPARAM,LPARAM,
    win16_dialog_classic_folder_context *active,void (*chrome)(HWND));
#endif
