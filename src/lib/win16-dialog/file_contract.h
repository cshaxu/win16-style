#ifndef WIN16_DIALOG_FILE_CONTRACT_H
#define WIN16_DIALOG_FILE_CONTRACT_H
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>
BOOL win16_dialog_file_prepare_a(const OPENFILENAMEA *,OPENFILENAMEA *,DWORD,LPOFNHOOKPROC);
BOOL win16_dialog_file_prepare_w(const OPENFILENAMEW *,OPENFILENAMEW *,DWORD,LPOFNHOOKPROC);
BOOL win16_dialog_file_resolve_links(BOOL,DWORD,BOOL,DWORD);
void win16_dialog_file_result_a(OPENFILENAMEA *,const OPENFILENAMEA *);
void win16_dialog_file_result_w(OPENFILENAMEW *,const OPENFILENAMEW *);
BOOL win16_dialog_com_can_bridge(HRESULT options_result,FILEOPENDIALOGOPTIONS flags,
    BOOL unsupported,BOOL has_events,BOOL save,BOOL experimental_events,BOOL experimental_hidden);
BOOL win16_dialog_copy_text(PCWSTR text,WCHAR **out);
BOOL win16_dialog_format_filters(UINT count,const COMDLG_FILTERSPEC *items,WCHAR **out);
DWORD win16_dialog_com_file_flags(FILEOPENDIALOGOPTIONS flags);
#endif
