#ifndef NTW16S_INTERCEPT_H
#define NTW16S_INTERCEPT_H
#include "ntwshk-dll/injection.h"
#include "lib-win32-style/application/dialog/observation.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"
extern ntwshk_context ntw16s_process_context;
classic_dialog_policy ntw16s_dialog_policy(void);
void ntw16s_observed_dialog_initialize(HWND window);
extern const style_dialog_windows ntw16s_observed_dialog_windows;
LONG ntw16s_attach_titles_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_processes_using(DWORD (*attach)(void *,void *));
void ntw16s_discard_processes(void);
LONG ntw16s_attach_shell_dialogs_using(DWORD (*attach)(void *,void *),BOOL observed_startup);
LONG ntw16s_register_file_dialogs(DWORD (*attach)(void *,void *));
LONG ntw16s_register_folder_dialogs(DWORD (*attach)(void *,void *));
#ifdef NTWSHK_UNIFIED
LONG ntw16s_attach_startup_processes(const ntwshk_context *unscoped);
LONG ntw16s_attach_startup_windows_using(DWORD (*attach)(void *,void *),void (*created)(HWND,DWORD));
LONG ntw16s_attach_startup_dialogs_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_startup_print_using(DWORD (*attach)(void *,void *));
LONG ntw16s_attach_startup_stock_using(DWORD (*attach)(void *,void *));
#endif
HRESULT ntw16s_dialog_style_scope(HRESULT (*callback)(void *),void *context);
void ntw16s_old_dialog_chrome(HWND window);
void ntw16s_font_dialog_chrome(HWND window);
#endif
