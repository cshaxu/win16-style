/* File-dialog COM identity and lifetime; caller supplies UI dispatch. */
#define COBJMACROS
#include "lib-style/common/classic_dialog_proxy.h"
#include "lib-style/common/classic_file_contract.h"
#include "lib-style/common/classic_file_helpers.h"
#include <dlgs.h>
#include <stdio.h>
#include <wchar.h>

void style_classic_dialog_proxy_capture_selection(HWND w,style_classic_dialog_proxy *p)
{
    HWND list=GetDlgItem(w,lst1);IShellItem *item=NULL;PWSTR folder=NULL;
    LRESULT selected=list?SendMessageW(list,LB_GETCURSEL,0,0):LB_ERR;
    LRESULT length=selected==LB_ERR?LB_ERR:SendMessageW(list,LB_GETTEXTLEN,(WPARAM)selected,0);
    if(length>=0 && length<MAX_PATH && p->live_folder &&
       SUCCEEDED(IShellItem_GetDisplayName(p->live_folder,SIGDN_FILESYSPATH,&folder))){
        WCHAR name[MAX_PATH],path[32768];
        if(SendMessageW(list,LB_GETTEXT,(WPARAM)selected,(LPARAM)name)!=LB_ERR){
            size_t n=wcslen(folder);int written=swprintf(path,ARRAYSIZE(path),L"%ls%ls%ls",folder,n && folder[n-1]==L'\\'?L"":L"\\",name);
            if(written>=0 && (size_t)written<ARRAYSIZE(path))(void)SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&item);
        }
    }
    if(folder)CoTaskMemFree(folder);
    if(p->live_selection)IShellItem_Release(p->live_selection);
    p->live_selection=item;
}

ULONG style_classic_dialog_proxy_add(style_classic_dialog_proxy *p){return (ULONG)InterlockedIncrement(&p->refs);}
ULONG style_classic_dialog_proxy_release(style_classic_dialog_proxy *p)
{
    LONG refs=InterlockedDecrement(&p->refs);
    if(!refs){style_dialog_events_release(p->native,&p->events);if(p->result)IShellItem_Release(p->result);
        if(p->live_folder)IShellItem_Release(p->live_folder);
        if(p->live_selection)IShellItem_Release(p->live_selection);
        if(p->initial_default)IShellItem_Release(p->initial_default);
        if(p->results)IShellItemArray_Release(p->results);
        if(p->title)CoTaskMemFree(p->title);
        if(p->filter)CoTaskMemFree(p->filter);
        if(p->extension)CoTaskMemFree(p->extension);
        if(p->ok_label)CoTaskMemFree(p->ok_label);
        if(p->cancel_label)CoTaskMemFree(p->cancel_label);
        if(p->file_label)CoTaskMemFree(p->file_label);
        if(p->pending_folder)CoTaskMemFree(p->pending_folder);
        if(p->custom_native)IFileDialogCustomize_Release(p->custom_native);
        if(p->v2_native)IFileDialog2_Release(p->v2_native);
        if(p->ole_native)IOleWindow_Release(p->ole_native);
        IUnknown_Release(p->variant);IFileDialog_Release(p->native);HeapFree(GetProcessHeap(),0,p);}
    return (ULONG)refs;
}

static HRESULT identity_query(void *owner,REFIID id,void **out){return style_classic_dialog_proxy_query(owner,id,out);}
static ULONG identity_add(void *owner){return style_classic_dialog_proxy_add(owner);}
static ULONG identity_release(void *owner){return style_classic_dialog_proxy_release(owner);}
static const style_dialog_identity_services identity_services={identity_query,identity_add,identity_release};

HRESULT style_classic_dialog_proxy_query(style_classic_dialog_proxy *p,REFIID id,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;
    if(IsEqualIID(id,&IID_IUnknown) || IsEqualIID(id,&IID_IModalWindow) || IsEqualIID(id,&IID_IFileDialog) ||
       (!p->save && IsEqualIID(id,&IID_IFileOpenDialog)) || (p->save && IsEqualIID(id,&IID_IFileSaveDialog))){
        *out=p;style_classic_dialog_proxy_add(p);return S_OK;
    }
    if(IsEqualIID(id,&IID_IOleWindow)){
        if(!p->ole_native){HRESULT hr=IFileDialog_QueryInterface(p->native,id,(void **)&p->ole_native);if(FAILED(hr))return hr;}
        style_dialog_ole_window_init(&p->ole,p,p->ole_native,&p->active_window,&identity_services);
        *out=&p->ole.iface;style_classic_dialog_proxy_add(p);return S_OK;
    }
    if(IsEqualIID(id,&IID_IFileDialog2)){
        if(!p->v2_native){HRESULT hr=IFileDialog_QueryInterface(p->native,id,(void **)&p->v2_native);if(FAILED(hr))return hr;}
        style_dialog_v2_init(&p->v2,p,p->v2_native,&p->unsupported,&p->cancel_label,&identity_services,p->dispatch);
        *out=&p->v2.iface;style_classic_dialog_proxy_add(p);return S_OK;
    }
    if(IsEqualIID(id,&IID_IFileDialogCustomize)){
        if(!p->custom_native){HRESULT hr=IFileDialog_QueryInterface(p->native,id,(void **)&p->custom_native);if(FAILED(hr))return hr;}
        style_dialog_customize_init(&p->custom,p,p->custom_native,&p->unsupported,&identity_services);
        *out=&p->custom.iface;style_classic_dialog_proxy_add(p);return S_OK;
    }
    HRESULT hr=IFileDialog_QueryInterface(p->native,id,out);
    /* A failed capability probe exposes no native interface and must not
     * disable bridging. Successful unknown interfaces can bypass the proxy. */
    if(SUCCEEDED(hr))p->unsupported=TRUE;
    return hr;
}

style_classic_dialog_proxy *style_classic_dialog_proxy_new(IUnknown *original,BOOL save,style_dialog_v2_call dispatch,BOOL experimental_events)
{
    IFileDialog *native=NULL;IUnknown *variant=NULL;
    if(FAILED(IUnknown_QueryInterface(original,&IID_IFileDialog,(void **)&native)))return NULL;
    if(FAILED(IUnknown_QueryInterface(original,save?&IID_IFileSaveDialog:&IID_IFileOpenDialog,(void **)&variant))){IFileDialog_Release(native);return NULL;}
    style_classic_dialog_proxy *p=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*p));
    if(!p){IUnknown_Release(variant);IFileDialog_Release(native);return NULL;}
    style_dialog_primary_init(&p->primary,p,variant,&p->unsupported,save,&identity_services,dispatch);
    p->save=save;p->refs=1;p->native=native;p->variant=variant;p->dispatch=dispatch;
    if(experimental_events)(void)IFileDialog_GetFolder(native,&p->initial_default);
    return p;
}

HRESULT style_classic_dialog_proxy_result(style_classic_dialog_proxy *p,IShellItem **item)
{
    if(!item)return E_POINTER;
    if(!p->completed)return IFileDialog_GetResult(p->native,item);
    if(p->results){*item=NULL;return E_UNEXPECTED;}
    *item=p->result;if(!*item)return E_FAIL;IShellItem_AddRef(*item);return S_OK;
}

HRESULT style_classic_dialog_proxy_get_folder(style_classic_dialog_proxy *p,IShellItem **item)
{
    if(!p->active_window)return IFileDialog_GetFolder(p->native,item);
    if(!item)return E_POINTER;
    *item=NULL;if(FAILED(p->live_folder_error))return p->live_folder_error;
    if(!p->live_folder)return E_FAIL;
    *item=p->live_folder;IShellItem_AddRef(*item);return S_OK;
}

HRESULT style_classic_dialog_proxy_get_selection(style_classic_dialog_proxy *p,IShellItem **item)
{
    if(!p->active_window)return IFileDialog_GetCurrentSelection(p->native,item);
    if(!item)return E_POINTER;
    *item=p->live_selection;if(!*item)return E_FAIL;IShellItem_AddRef(*item);return S_OK;
}
HRESULT style_classic_dialog_proxy_close(style_classic_dialog_proxy *p,HRESULT result)
{
    HWND w=p->active_window;if(!w)return IFileDialog_Close(p->native,result);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId())return RPC_E_WRONG_THREAD;
    if(!PostMessageW(w,WM_CLOSE,0,0)){DWORD error=GetLastError();return HRESULT_FROM_WIN32(error?error:ERROR_INVALID_FUNCTION);}
    p->classic_close_pending=TRUE;p->classic_close_result=result;return S_OK;
}

HRESULT style_classic_dialog_proxy_title(style_classic_dialog_proxy *p,LPCWSTR text)
{
    HRESULT hr=IFileDialog_SetTitle(p->native,text);if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(!style_dialog_copy_text(text,&copy)){p->unsupported=TRUE;return E_OUTOFMEMORY;}
    if(p->title)CoTaskMemFree(p->title);
    p->title=copy;return hr;
}
/* Capture only data accepted by the native object; allocation failure leaves
 * its complete state available for a native-dialog fallback. */
HRESULT style_classic_dialog_proxy_label(style_classic_dialog_proxy *p,LPCWSTR text,BOOL ok_button)
{
    HRESULT hr=ok_button?IFileDialog_SetOkButtonLabel(p->native,text):IFileDialog_SetFileNameLabel(p->native,text);
    if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(!style_dialog_copy_text(text,&copy)){p->unsupported=TRUE;return hr;}
    WCHAR **slot=ok_button?&p->ok_label:&p->file_label;if(*slot)CoTaskMemFree(*slot);*slot=copy;return hr;
}
HRESULT style_classic_dialog_proxy_types(style_classic_dialog_proxy *p,UINT count,const COMDLG_FILTERSPEC *items)
{
    HRESULT hr=IFileDialog_SetFileTypes(p->native,count,items);if(FAILED(hr))return hr;
    WCHAR *filter=NULL;
    if(!style_dialog_format_filters(count,items,&filter)){p->unsupported=TRUE;return hr;}
    if(p->filter)CoTaskMemFree(p->filter);
    p->filter=filter;p->filter_index=1;return hr;
}
HRESULT style_classic_dialog_proxy_extension(style_classic_dialog_proxy *p,LPCWSTR value)
{
    HRESULT hr=IFileDialog_SetDefaultExtension(p->native,value);if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(!style_dialog_copy_text(value,&copy)){p->unsupported=TRUE;return hr;}
    if(p->extension)CoTaskMemFree(p->extension);
    p->extension=copy;return hr;
}
void style_classic_dialog_proxy_capture_folder(style_classic_dialog_proxy *p,BOOL experimental_events)
{
    WCHAR path[MAX_PATH];IShellItem *item=NULL;
    HRESULT hr=style_classic_file_folder(p->active_window,path,ARRAYSIZE(path))?
        SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&item):E_FAIL;
    if(p->live_folder)IShellItem_Release(p->live_folder);
    p->live_folder=item;p->live_folder_error=hr;
    if(experimental_events && p->live_selection){IShellItem_Release(p->live_selection);p->live_selection=NULL;}
}

HRESULT style_classic_dialog_proxy_set_folder(style_classic_dialog_proxy *p,IShellItem *item,PCWSTR message_name)
{
    HWND w=p->active_window;
    if(!w)return IFileDialog_SetFolder(p->native,item);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId())return RPC_E_WRONG_THREAD;
    if(!item)return E_INVALIDARG;
    PWSTR path=NULL;HRESULT hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
    if(FAILED(hr))return hr;
    DWORD attrs=GetFileAttributesW(path);
    if(attrs==INVALID_FILE_ATTRIBUTES || !(attrs&FILE_ATTRIBUTE_DIRECTORY)){
        DWORD error=attrs==INVALID_FILE_ATTRIBUTES?GetLastError():ERROR_DIRECTORY;
        CoTaskMemFree(path);return HRESULT_FROM_WIN32(error?error:ERROR_INVALID_DATA);
    }
    UINT message=RegisterWindowMessageW(message_name);
    if(!message){DWORD error=GetLastError();CoTaskMemFree(path);return HRESULT_FROM_WIN32(error?error:ERROR_INVALID_FUNCTION);}
    hr=IFileDialog_SetFolder(p->native,item);
    if(FAILED(hr)){CoTaskMemFree(path);return hr;}
    if(p->active_window!=w || !IsWindow(w)){CoTaskMemFree(path);return E_UNEXPECTED;}
    WCHAR *previous=p->pending_folder;p->pending_folder=path;
    if(!PostMessageW(w,message,0,0)){
        DWORD error=GetLastError();p->pending_folder=previous;CoTaskMemFree(path);
        return HRESULT_FROM_WIN32(error?error:ERROR_INVALID_FUNCTION);
    }
    if(previous)CoTaskMemFree(previous);
    return hr;
}

void style_classic_dialog_proxy_navigate_contents(HWND w,style_classic_dialog_proxy *p,PCWSTR key,PCWSTR folder,
    WCHAR **owned_filename,BOOL experimental_events)
{
    HWND edit=GetDlgItem(w,edt1);int length=edit?GetWindowTextLengthW(edit):-1;
    WCHAR *filename=length>=0 && length<32768?CoTaskMemAlloc(((size_t)length+1)*sizeof(WCHAR)):NULL;
    *owned_filename=filename;
    if(filename){
        GetWindowTextW(edit,filename,length+1);
        DWORD attrs=GetFileAttributesW(folder);
        if(attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY) && SetWindowTextW(edit,folder)){
            if(experimental_events)p->classic_navigating=TRUE;
            SendMessageW(w,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(w,IDOK));
            if(experimental_events && IsWindow(w) && GetPropW(w,key)==p)p->classic_navigating=FALSE;
            if(IsWindow(w) && GetPropW(w,key)==p && IsWindow(edit))SetWindowTextW(edit,filename);
        }
    }
}
