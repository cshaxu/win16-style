#ifndef NTW16S_INTERCEPT_H
#define NTW16S_INTERCEPT_H
#include "injection.h"
extern ntw16s_context ntw16s_process_context;
LONG ntw16s_attach_titles(void); /* joins the current early Detours transaction */
LONG ntw16s_attach_processes(void);
LONG ntw16s_attach_file_dialogs(void);
LONG ntw16s_attach_com_dialogs(void);
LONG ntw16s_attach_folder_dialogs(void);
BOOL ntw16s_modern_dialog_active(void);
HRESULT ntw16s_dialog_style_scope(HRESULT (*callback)(void *),void *context);
HRESULT ntw16s_classic_folder(HWND owner,PCWSTR title,PCWSTR initial,PCWSTR ok_label,PCWSTR cancel_label,PCWSTR file_label,PWSTR result,DWORD capacity);
void ntw16s_old_dialog_chrome(HWND window);
void ntw16s_font_dialog_chrome(HWND window);
#endif
