#ifndef STYLE_CLASSIC_FILE_CONTRACT_H
#define STYLE_CLASSIC_FILE_CONTRACT_H
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>
BOOL style_classic_file_prepare_a(const OPENFILENAMEA *,OPENFILENAMEA *,DWORD,LPOFNHOOKPROC);
BOOL style_classic_file_prepare_w(const OPENFILENAMEW *,OPENFILENAMEW *,DWORD,LPOFNHOOKPROC);
BOOL style_classic_file_resolve_links(BOOL,DWORD,BOOL,DWORD);
void style_classic_file_result_a(OPENFILENAMEA *,const OPENFILENAMEA *);
void style_classic_file_result_w(OPENFILENAMEW *,const OPENFILENAMEW *);
BOOL style_classic_com_can_bridge(HRESULT options_result,FILEOPENDIALOGOPTIONS flags,
    BOOL unsupported,BOOL has_events,BOOL save,BOOL experimental_events,BOOL experimental_hidden);
BOOL style_dialog_copy_text(PCWSTR text,WCHAR **out);
BOOL style_dialog_format_filters(UINT count,const COMDLG_FILTERSPEC *items,WCHAR **out);
DWORD style_classic_com_file_flags(FILEOPENDIALOGOPTIONS flags);
#endif
