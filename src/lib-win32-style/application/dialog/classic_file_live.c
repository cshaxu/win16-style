/* Reflect live values between the native COM object and classic controls. */
#define COBJMACROS
#include "lib-win32-style/application/dialog/classic_file_live.h"
#include <dlgs.h>

HRESULT style_classic_file_live_set_filter(style_classic_dialog_proxy *proxy,UINT index)
{
    HRESULT hr=IFileDialog_SetFileTypeIndex(proxy->native,index);if(SUCCEEDED(hr))proxy->filter_index=index;return hr;
}
HRESULT style_classic_file_live_get_filter(style_classic_dialog_proxy *proxy,UINT *index)
{
    if(!proxy->active_window || !proxy->filter || !index)return IFileDialog_GetFileTypeIndex(proxy->native,index);
    HWND combo=GetDlgItem(proxy->active_window,cmb1);
    if(!combo)return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    LRESULT selected=SendMessageW(combo,CB_GETCURSEL,0,0);
    if(selected==CB_ERR)return E_FAIL;
    *index=(UINT)selected+1;return S_OK;
}

HRESULT style_classic_file_live_set_name(style_classic_dialog_proxy *proxy,LPCWSTR name)
{
    HRESULT hr=IFileDialog_SetFileName(proxy->native,name);
    if(FAILED(hr) || !proxy->active_window)return hr;
    HWND edit=GetDlgItem(proxy->active_window,edt1);
    if(!edit || !SetWindowTextW(edit,name))return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    return hr;
}
HRESULT style_classic_file_live_get_name(style_classic_dialog_proxy *proxy,LPWSTR *name)
{
    if(!proxy->active_window)return IFileDialog_GetFileName(proxy->native,name);
    if(!name)return E_POINTER;
    *name=NULL;HWND edit=GetDlgItem(proxy->active_window,edt1);
    if(!edit)return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    int length=GetWindowTextLengthW(edit);
    if(length<0 || length>=32768)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    WCHAR *value=CoTaskMemAlloc(((size_t)length+1)*sizeof(WCHAR));
    if(!value)return E_OUTOFMEMORY;
    GetWindowTextW(edit,value,length+1);*name=value;return S_OK;
}
