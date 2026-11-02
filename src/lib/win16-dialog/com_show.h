#ifndef WIN16_DIALOG_COM_SHOW_H
#define WIN16_DIALOG_COM_SHOW_H
#include <windows.h>
#include <shobjidl.h>
#include <commdlg.h>
typedef struct win16_dialog_com_show_state {
    void *owner; IFileDialog *native; BOOL save;
    BOOL *unsupported,*completed; IShellItem **result; IShellItemArray **results;
    WCHAR **title,**filter,**extension,**ok_label,**cancel_label,**file_label;
    UINT *filter_index; LPOFNHOOKPROC classic_hook;
    BOOL experimental_events,experimental_hidden; BOOL *classic_hidden,*classic_close_pending;
    HRESULT *classic_close_result;
    BOOL (*session_active)(void);
    BOOL (*has_events)(void *);
    HRESULT (*show_native)(void *,HWND);
    HRESULT (*scope)(HRESULT (*)(void *),void *);
} win16_dialog_com_show_state;
HRESULT win16_dialog_com_show(win16_dialog_com_show_state *state,HWND owner);
#endif
