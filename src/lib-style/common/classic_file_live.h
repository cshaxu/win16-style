#ifndef STYLE_CLASSIC_FILE_LIVE_H
#define STYLE_CLASSIC_FILE_LIVE_H
#include <windows.h>
#include <shobjidl.h>
HRESULT style_classic_file_live_set_filter(IFileDialog *,UINT *,UINT);
HRESULT style_classic_file_live_get_filter(IFileDialog *,HWND *,WCHAR **,UINT *);
HRESULT style_classic_file_live_set_name(IFileDialog *,HWND *,LPCWSTR);
HRESULT style_classic_file_live_get_name(IFileDialog *,HWND *,LPWSTR *);
#endif
