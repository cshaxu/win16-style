/* IFileDialog2 facade; owner supplies stateful operations and COM identity. */
#define COBJMACROS
#include "v2.h"
#include <wchar.h>
#include <string.h>
#define V2_CALL(p,op,a,b) ((p)->call((p)->owner,V2_##op,(ULONG_PTR)(a),(ULONG_PTR)(b)))

static HRESULT STDMETHODCALLTYPE v2_QueryInterface(IFileDialog2 *This, REFIID riid, void **ppvObject)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return p->identity->query(p->owner,riid,ppvObject);
}

static ULONG STDMETHODCALLTYPE v2_AddRef(IFileDialog2 *This)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return p->identity->add(p->owner);
}

static ULONG STDMETHODCALLTYPE v2_Release(IFileDialog2 *This)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return p->identity->release(p->owner);
}

static HRESULT STDMETHODCALLTYPE v2_Show(IFileDialog2 *This, HWND hwndOwner)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,SHOW,hwndOwner,0);
}

static HRESULT STDMETHODCALLTYPE v2_SetFileTypes(IFileDialog2 *This, UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,TYPES,cFileTypes,rgFilterSpec);
}

static HRESULT STDMETHODCALLTYPE v2_SetFileTypeIndex(IFileDialog2 *This, UINT iFileType)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,SET_INDEX,iFileType,0);
}

static HRESULT STDMETHODCALLTYPE v2_GetFileTypeIndex(IFileDialog2 *This, UINT *piFileType)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,GET_INDEX,piFileType,0);
}

static HRESULT STDMETHODCALLTYPE v2_Advise(IFileDialog2 *This, IFileDialogEvents *pfde, DWORD *pdwCookie)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,ADVISE,pfde,pdwCookie);
}

static HRESULT STDMETHODCALLTYPE v2_Unadvise(IFileDialog2 *This, DWORD dwCookie)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,UNADVISE,dwCookie,0);
}

static HRESULT STDMETHODCALLTYPE v2_SetOptions(IFileDialog2 *This, FILEOPENDIALOGOPTIONS fos)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return IFileDialog2_SetOptions(p->native ,fos);
}

static HRESULT STDMETHODCALLTYPE v2_GetOptions(IFileDialog2 *This, FILEOPENDIALOGOPTIONS *pfos)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return IFileDialog2_GetOptions(p->native ,pfos);
}

static HRESULT STDMETHODCALLTYPE v2_SetDefaultFolder(IFileDialog2 *This, IShellItem *psi)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return IFileDialog2_SetDefaultFolder(p->native ,psi);
}

static HRESULT STDMETHODCALLTYPE v2_SetFolder(IFileDialog2 *This, IShellItem *psi)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,SET_FOLDER,psi,0);
}

static HRESULT STDMETHODCALLTYPE v2_GetFolder(IFileDialog2 *This, IShellItem **ppsi)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,GET_FOLDER,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE v2_GetCurrentSelection(IFileDialog2 *This, IShellItem **ppsi)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,GET_SELECTION,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE v2_SetFileName(IFileDialog2 *This, LPCWSTR pszName)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,SET_NAME,pszName,0);
}

static HRESULT STDMETHODCALLTYPE v2_GetFileName(IFileDialog2 *This, LPWSTR *pszName)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,GET_NAME,pszName,0);
}

static HRESULT STDMETHODCALLTYPE v2_SetTitle(IFileDialog2 *This, LPCWSTR pszTitle)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,TITLE,pszTitle,0);
}

static HRESULT STDMETHODCALLTYPE v2_SetOkButtonLabel(IFileDialog2 *This, LPCWSTR pszText)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,LABEL,pszText,TRUE);
}

static HRESULT STDMETHODCALLTYPE v2_SetFileNameLabel(IFileDialog2 *This, LPCWSTR pszLabel)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,LABEL,pszLabel,FALSE);
}

static HRESULT STDMETHODCALLTYPE v2_GetResult(IFileDialog2 *This, IShellItem **ppsi)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,RESULT,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE v2_AddPlace(IFileDialog2 *This, IShellItem *psi, FDAP fdap)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;*p->unsupported=TRUE;return IFileDialog2_AddPlace(p->native ,psi,fdap);
}

static HRESULT STDMETHODCALLTYPE v2_SetDefaultExtension(IFileDialog2 *This, LPCWSTR pszDefaultExtension)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,EXTENSION,pszDefaultExtension,0);
}

static HRESULT STDMETHODCALLTYPE v2_Close(IFileDialog2 *This, HRESULT hr)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;return V2_CALL(p,CLOSE,hr,0);
}

static HRESULT STDMETHODCALLTYPE v2_SetClientGuid(IFileDialog2 *This, REFGUID guid)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;*p->unsupported=TRUE;return IFileDialog2_SetClientGuid(p->native ,guid);
}

static HRESULT STDMETHODCALLTYPE v2_ClearClientData(IFileDialog2 *This)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;*p->unsupported=TRUE;return IFileDialog2_ClearClientData(p->native);
}

static HRESULT STDMETHODCALLTYPE v2_SetFilter(IFileDialog2 *This, IShellItemFilter *pFilter)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;*p->unsupported=TRUE;return IFileDialog2_SetFilter(p->native ,pFilter);
}

static HRESULT STDMETHODCALLTYPE v2_SetCancelButtonLabel(IFileDialog2 *This, LPCWSTR pszLabel)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;
    HRESULT hr=IFileDialog2_SetCancelButtonLabel(p->native,pszLabel);if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(pszLabel){size_t n=wcslen(pszLabel)+1;copy=CoTaskMemAlloc(n*sizeof(WCHAR));
        if(!copy){*p->unsupported=TRUE;return hr;}memcpy(copy,pszLabel,n*sizeof(WCHAR));}
    if(*p->cancel_label)CoTaskMemFree(*p->cancel_label);
    *p->cancel_label=copy;return hr;
}

static HRESULT STDMETHODCALLTYPE v2_SetNavigationRoot(IFileDialog2 *This, IShellItem *psi)
{
    win16_dialog_v2 *p=(win16_dialog_v2 *)This;*p->unsupported=TRUE;return IFileDialog2_SetNavigationRoot(p->native ,psi);
}

static IFileDialog2Vtbl v2_vtable={
    v2_QueryInterface,
    v2_AddRef,
    v2_Release,
    v2_Show,
    v2_SetFileTypes,
    v2_SetFileTypeIndex,
    v2_GetFileTypeIndex,
    v2_Advise,
    v2_Unadvise,
    v2_SetOptions,
    v2_GetOptions,
    v2_SetDefaultFolder,
    v2_SetFolder,
    v2_GetFolder,
    v2_GetCurrentSelection,
    v2_SetFileName,
    v2_GetFileName,
    v2_SetTitle,
    v2_SetOkButtonLabel,
    v2_SetFileNameLabel,
    v2_GetResult,
    v2_AddPlace,
    v2_SetDefaultExtension,
    v2_Close,
    v2_SetClientGuid,
    v2_ClearClientData,
    v2_SetFilter,
    v2_SetCancelButtonLabel,
    v2_SetNavigationRoot
};

void win16_dialog_v2_init(win16_dialog_v2 *adapter,void *owner,IFileDialog2 *native,BOOL *unsupported,
    WCHAR **cancel_label,const win16_dialog_customize_services *identity,win16_dialog_v2_call call)
{
    adapter->iface.lpVtbl=&v2_vtable;adapter->owner=owner;adapter->native=native;
    adapter->unsupported=unsupported;adapter->cancel_label=cancel_label;adapter->identity=identity;adapter->call=call;
}
