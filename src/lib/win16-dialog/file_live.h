#ifndef WIN16_DIALOG_FILE_LIVE_H
#define WIN16_DIALOG_FILE_LIVE_H
#include <windows.h>
#include <shobjidl.h>
HRESULT win16_dialog_live_set_filter(IFileDialog *,UINT *,UINT);
HRESULT win16_dialog_live_get_filter(IFileDialog *,HWND *,WCHAR **,UINT *);
HRESULT win16_dialog_live_set_name(IFileDialog *,HWND *,LPCWSTR);
HRESULT win16_dialog_live_get_name(IFileDialog *,HWND *,LPWSTR *);
#endif
