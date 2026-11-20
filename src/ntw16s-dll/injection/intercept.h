#ifndef NTW16S_INTERCEPT_H
#define NTW16S_INTERCEPT_H
#include "ntwshk-dll/injection.h"
#include "lib-win32-style/application/dialog/observation.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"
extern ntwshk_context ntw16s_process_context;
BOOL ntw16s_process_session_active(void);
classic_dialog_policy ntw16s_dialog_policy(void);
void ntw16s_observed_dialog_initialize(HWND window);
extern const style_dialog_windows ntw16s_observed_dialog_windows;
LONG ntw16s_attach_titles(void); /* joins the current startup transaction */
LONG ntw16s_attach_titles_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_stock_dialogs_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_processes(void);
LONG ntw16s_attach_processes_using(DWORD (*attach)(void *,void *));
void ntw16s_discard_processes(void);
LONG ntw16s_attach_file_dialogs_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_com_dialogs_using(DWORD (*attach)(void *,void *));
#ifdef NTWSHK_UNIFIED
void ntw2ks_dialog_initialize(HWND window);
LONG ntw16s_attach_win2k_file_dialogs_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_win2k_folder_dialogs_using(DWORD (*attach)(void *,void *));
DWORD ntw16s_attach_win2k_com_dialogs_using(DWORD (*attach)(void *,void *));
#endif
LONG ntw16s_attach_folder_dialogs_using(DWORD (*attach)(void *,void *));
HRESULT ntw16s_dialog_style_scope(HRESULT (*callback)(void *),void *context);
void ntw16s_old_dialog_chrome(HWND window);
void ntw16s_font_dialog_chrome(HWND window);
#endif
