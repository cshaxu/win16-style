/* Reflect live values between the native COM object and classic controls. */
#define COBJMACROS
#include "lib-style/common/classic_file_live.h"
#include <dlgs.h>

HRESULT style_classic_file_live_set_filter(IFileDialog *native,UINT *filter_index,UINT index)
{
    HRESULT hr=IFileDialog_SetFileTypeIndex(native,index);if(SUCCEEDED(hr))*filter_index=index;return hr;
}
HRESULT style_classic_file_live_get_filter(IFileDialog *native,HWND *active_window,WCHAR **filter,UINT *index)
{
    if(!*active_window || !*filter || !index)return IFileDialog_GetFileTypeIndex(native,index);
    HWND combo=GetDlgItem(*active_window,cmb1);
    if(!combo)return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    LRESULT selected=SendMessageW(combo,CB_GETCURSEL,0,0);
    if(selected==CB_ERR)return E_FAIL;
    *index=(UINT)selected+1;return S_OK;
}

HRESULT style_classic_file_live_set_name(IFileDialog *native,HWND *active_window,LPCWSTR name)
{
    HRESULT hr=IFileDialog_SetFileName(native,name);
    if(FAILED(hr) || !*active_window)return hr;
    HWND edit=GetDlgItem(*active_window,edt1);
    if(!edit || !SetWindowTextW(edit,name))return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    return hr;
}
HRESULT style_classic_file_live_get_name(IFileDialog *native,HWND *active_window,LPWSTR *name)
{
    if(!*active_window)return IFileDialog_GetFileName(native,name);
    if(!name)return E_POINTER;
    *name=NULL;HWND edit=GetDlgItem(*active_window,edt1);
    if(!edit)return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    int length=GetWindowTextLengthW(edit);
    if(length<0 || length>=32768)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    WCHAR *value=CoTaskMemAlloc(((size_t)length+1)*sizeof(WCHAR));
    if(!value)return E_OUTOFMEMORY;
    GetWindowTextW(edit,value,length+1);*name=value;return S_OK;
}
