#ifndef NTW16S_INTERCEPT_H
#define NTW16S_INTERCEPT_H
#include "ntwshk-dll/injection.h"
extern ntwshk_context ntw16s_process_context;
BOOL ntw16s_process_session_active(void);
LONG ntw16s_attach_titles(void); /* joins the current early Detours transaction */
LONG ntw16s_attach_titles_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_processes(void);
LONG ntw16s_attach_processes_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_file_dialogs(void);
LONG ntw16s_attach_file_dialogs_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_com_dialogs(void);
LONG ntw16s_attach_com_dialogs_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_folder_dialogs(void);
LONG ntw16s_attach_folder_dialogs_using(DWORD (*attach)(void *,void *));
BOOL ntw16s_modern_dialog_active(void);
HRESULT ntw16s_dialog_style_scope(HRESULT (*callback)(void *),void *context);
BOOL ntw16s_old_dialog_folder(HWND window,PWSTR path,size_t capacity);
void ntw16s_old_dialog_chrome(HWND window);
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
struct ntw16s_creation_scope;
struct ntw16s_creation_scope *ntw16s_creation_begin_dialog(BOOL enabled);
void (*ntw16s_creation_dialog_observer(void (*observer)(HWND)))(HWND);
#endif
void ntw16s_font_dialog_chrome(HWND window);
#endif
