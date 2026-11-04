/* IFileOpenDialog and IFileSaveDialog facades; owner supplies state and identity. */
#define COBJMACROS
#include "primary.h"
#define PRIMARY_CALL(p,op,a,b) ((p)->call((p)->owner,V2_##op,(ULONG_PTR)(a),(ULONG_PTR)(b)))

static HRESULT STDMETHODCALLTYPE open_QueryInterface(IFileOpenDialog *This, REFIID riid, void **ppvObject)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return p->identity->query(p->owner,riid,ppvObject);
}

static ULONG STDMETHODCALLTYPE open_AddRef(IFileOpenDialog *This)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return p->identity->add(p->owner);
}

static ULONG STDMETHODCALLTYPE open_Release(IFileOpenDialog *This)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return p->identity->release(p->owner);
}

static HRESULT STDMETHODCALLTYPE open_Show(IFileOpenDialog *This, HWND hwndOwner)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,SHOW,hwndOwner,0);
}

static HRESULT STDMETHODCALLTYPE open_SetFileTypes(IFileOpenDialog *This, UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,TYPES,cFileTypes,rgFilterSpec);
}

static HRESULT STDMETHODCALLTYPE open_SetFileTypeIndex(IFileOpenDialog *This, UINT iFileType)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,SET_INDEX,iFileType,0);
}

static HRESULT STDMETHODCALLTYPE open_GetFileTypeIndex(IFileOpenDialog *This, UINT *piFileType)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,GET_INDEX,piFileType,0);
}

static HRESULT STDMETHODCALLTYPE open_Advise(IFileOpenDialog *This, IFileDialogEvents *pfde, DWORD *pdwCookie)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,ADVISE,pfde,pdwCookie);
}

static HRESULT STDMETHODCALLTYPE open_Unadvise(IFileOpenDialog *This, DWORD dwCookie)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,UNADVISE,dwCookie,0);
}

static HRESULT STDMETHODCALLTYPE open_SetOptions(IFileOpenDialog *This, FILEOPENDIALOGOPTIONS fos)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return IFileOpenDialog_SetOptions((IFileOpenDialog *)p->variant ,fos);
}

static HRESULT STDMETHODCALLTYPE open_GetOptions(IFileOpenDialog *This, FILEOPENDIALOGOPTIONS *pfos)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return IFileOpenDialog_GetOptions((IFileOpenDialog *)p->variant ,pfos);
}

static HRESULT STDMETHODCALLTYPE open_SetDefaultFolder(IFileOpenDialog *This, IShellItem *psi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return IFileOpenDialog_SetDefaultFolder((IFileOpenDialog *)p->variant ,psi);
}

static HRESULT STDMETHODCALLTYPE open_SetFolder(IFileOpenDialog *This, IShellItem *psi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,SET_FOLDER,psi,0);
}

static HRESULT STDMETHODCALLTYPE open_GetFolder(IFileOpenDialog *This, IShellItem **ppsi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,GET_FOLDER,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE open_GetCurrentSelection(IFileOpenDialog *This, IShellItem **ppsi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,PRIMARY_GET_SELECTION,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE open_SetFileName(IFileOpenDialog *This, LPCWSTR pszName)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,SET_NAME,pszName,0);
}

static HRESULT STDMETHODCALLTYPE open_GetFileName(IFileOpenDialog *This, LPWSTR *pszName)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,GET_NAME,pszName,0);
}

static HRESULT STDMETHODCALLTYPE open_SetTitle(IFileOpenDialog *This, LPCWSTR pszTitle)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,TITLE,pszTitle,0);
}

static HRESULT STDMETHODCALLTYPE open_SetOkButtonLabel(IFileOpenDialog *This, LPCWSTR pszText)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,LABEL,pszText,TRUE);
}

static HRESULT STDMETHODCALLTYPE open_SetFileNameLabel(IFileOpenDialog *This, LPCWSTR pszLabel)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,LABEL,pszLabel,FALSE);
}

static HRESULT STDMETHODCALLTYPE open_GetResult(IFileOpenDialog *This, IShellItem **ppsi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,RESULT,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE open_AddPlace(IFileOpenDialog *This, IShellItem *psi, FDAP fdap)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileOpenDialog_AddPlace((IFileOpenDialog *)p->variant ,psi,fdap);
}

static HRESULT STDMETHODCALLTYPE open_SetDefaultExtension(IFileOpenDialog *This, LPCWSTR pszDefaultExtension)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,EXTENSION,pszDefaultExtension,0);
}

static HRESULT STDMETHODCALLTYPE open_Close(IFileOpenDialog *This, HRESULT hr)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,PRIMARY_CLOSE,hr,0);
}

static HRESULT STDMETHODCALLTYPE open_SetClientGuid(IFileOpenDialog *This, REFGUID guid)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileOpenDialog_SetClientGuid((IFileOpenDialog *)p->variant ,guid);
}

static HRESULT STDMETHODCALLTYPE open_ClearClientData(IFileOpenDialog *This)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileOpenDialog_ClearClientData((IFileOpenDialog *)p->variant);
}

static HRESULT STDMETHODCALLTYPE open_SetFilter(IFileOpenDialog *This, IShellItemFilter *pFilter)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileOpenDialog_SetFilter((IFileOpenDialog *)p->variant ,pFilter);
}

static HRESULT STDMETHODCALLTYPE open_GetResults(IFileOpenDialog *This, IShellItemArray **ppenum)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,PRIMARY_GET_RESULTS,ppenum,0);
}

static HRESULT STDMETHODCALLTYPE open_GetSelectedItems(IFileOpenDialog *This, IShellItemArray **ppsai)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileOpenDialog_GetSelectedItems((IFileOpenDialog *)p->variant ,ppsai);
}

static const IFileOpenDialogVtbl open_vtable={
    open_QueryInterface,
    open_AddRef,
    open_Release,
    open_Show,
    open_SetFileTypes,
    open_SetFileTypeIndex,
    open_GetFileTypeIndex,
    open_Advise,
    open_Unadvise,
    open_SetOptions,
    open_GetOptions,
    open_SetDefaultFolder,
    open_SetFolder,
    open_GetFolder,
    open_GetCurrentSelection,
    open_SetFileName,
    open_GetFileName,
    open_SetTitle,
    open_SetOkButtonLabel,
    open_SetFileNameLabel,
    open_GetResult,
    open_AddPlace,
    open_SetDefaultExtension,
    open_Close,
    open_SetClientGuid,
    open_ClearClientData,
    open_SetFilter,
    open_GetResults,
    open_GetSelectedItems
};

static HRESULT STDMETHODCALLTYPE save_QueryInterface(IFileSaveDialog *This, REFIID riid, void **ppvObject)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return p->identity->query(p->owner,riid,ppvObject);
}

static ULONG STDMETHODCALLTYPE save_AddRef(IFileSaveDialog *This)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return p->identity->add(p->owner);
}

static ULONG STDMETHODCALLTYPE save_Release(IFileSaveDialog *This)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return p->identity->release(p->owner);
}

static HRESULT STDMETHODCALLTYPE save_Show(IFileSaveDialog *This, HWND hwndOwner)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,SHOW,hwndOwner,0);
}

static HRESULT STDMETHODCALLTYPE save_SetFileTypes(IFileSaveDialog *This, UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,TYPES,cFileTypes,rgFilterSpec);
}

static HRESULT STDMETHODCALLTYPE save_SetFileTypeIndex(IFileSaveDialog *This, UINT iFileType)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,SET_INDEX,iFileType,0);
}

static HRESULT STDMETHODCALLTYPE save_GetFileTypeIndex(IFileSaveDialog *This, UINT *piFileType)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,GET_INDEX,piFileType,0);
}

static HRESULT STDMETHODCALLTYPE save_Advise(IFileSaveDialog *This, IFileDialogEvents *pfde, DWORD *pdwCookie)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,ADVISE,pfde,pdwCookie);
}

static HRESULT STDMETHODCALLTYPE save_Unadvise(IFileSaveDialog *This, DWORD dwCookie)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,UNADVISE,dwCookie,0);
}

static HRESULT STDMETHODCALLTYPE save_SetOptions(IFileSaveDialog *This, FILEOPENDIALOGOPTIONS fos)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return IFileSaveDialog_SetOptions((IFileSaveDialog *)p->variant ,fos);
}

static HRESULT STDMETHODCALLTYPE save_GetOptions(IFileSaveDialog *This, FILEOPENDIALOGOPTIONS *pfos)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return IFileSaveDialog_GetOptions((IFileSaveDialog *)p->variant ,pfos);
}

static HRESULT STDMETHODCALLTYPE save_SetDefaultFolder(IFileSaveDialog *This, IShellItem *psi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return IFileSaveDialog_SetDefaultFolder((IFileSaveDialog *)p->variant ,psi);
}

static HRESULT STDMETHODCALLTYPE save_SetFolder(IFileSaveDialog *This, IShellItem *psi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,SET_FOLDER,psi,0);
}

static HRESULT STDMETHODCALLTYPE save_GetFolder(IFileSaveDialog *This, IShellItem **ppsi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,GET_FOLDER,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE save_GetCurrentSelection(IFileSaveDialog *This, IShellItem **ppsi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,PRIMARY_GET_SELECTION,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE save_SetFileName(IFileSaveDialog *This, LPCWSTR pszName)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,SET_NAME,pszName,0);
}

static HRESULT STDMETHODCALLTYPE save_GetFileName(IFileSaveDialog *This, LPWSTR *pszName)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,GET_NAME,pszName,0);
}

static HRESULT STDMETHODCALLTYPE save_SetTitle(IFileSaveDialog *This, LPCWSTR pszTitle)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,TITLE,pszTitle,0);
}

static HRESULT STDMETHODCALLTYPE save_SetOkButtonLabel(IFileSaveDialog *This, LPCWSTR pszText)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,LABEL,pszText,TRUE);
}

static HRESULT STDMETHODCALLTYPE save_SetFileNameLabel(IFileSaveDialog *This, LPCWSTR pszLabel)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,LABEL,pszLabel,FALSE);
}

static HRESULT STDMETHODCALLTYPE save_GetResult(IFileSaveDialog *This, IShellItem **ppsi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,RESULT,ppsi,0);
}

static HRESULT STDMETHODCALLTYPE save_AddPlace(IFileSaveDialog *This, IShellItem *psi, FDAP fdap)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_AddPlace((IFileSaveDialog *)p->variant ,psi,fdap);
}

static HRESULT STDMETHODCALLTYPE save_SetDefaultExtension(IFileSaveDialog *This, LPCWSTR pszDefaultExtension)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,EXTENSION,pszDefaultExtension,0);
}

static HRESULT STDMETHODCALLTYPE save_Close(IFileSaveDialog *This, HRESULT hr)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;return PRIMARY_CALL(p,PRIMARY_CLOSE,hr,0);
}

static HRESULT STDMETHODCALLTYPE save_SetClientGuid(IFileSaveDialog *This, REFGUID guid)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_SetClientGuid((IFileSaveDialog *)p->variant ,guid);
}

static HRESULT STDMETHODCALLTYPE save_ClearClientData(IFileSaveDialog *This)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_ClearClientData((IFileSaveDialog *)p->variant);
}

static HRESULT STDMETHODCALLTYPE save_SetFilter(IFileSaveDialog *This, IShellItemFilter *pFilter)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_SetFilter((IFileSaveDialog *)p->variant ,pFilter);
}

static HRESULT STDMETHODCALLTYPE save_SetSaveAsItem(IFileSaveDialog *This, IShellItem *psi)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_SetSaveAsItem((IFileSaveDialog *)p->variant ,psi);
}

static HRESULT STDMETHODCALLTYPE save_SetProperties(IFileSaveDialog *This, IPropertyStore *pStore)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_SetProperties((IFileSaveDialog *)p->variant ,pStore);
}

static HRESULT STDMETHODCALLTYPE save_SetCollectedProperties(IFileSaveDialog *This, IPropertyDescriptionList *pList, WINBOOL fAppendDefault)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_SetCollectedProperties((IFileSaveDialog *)p->variant ,pList,fAppendDefault);
}

static HRESULT STDMETHODCALLTYPE save_GetProperties(IFileSaveDialog *This, IPropertyStore **ppStore)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_GetProperties((IFileSaveDialog *)p->variant ,ppStore);
}

static HRESULT STDMETHODCALLTYPE save_ApplyProperties(IFileSaveDialog *This, IShellItem *psi, IPropertyStore *pStore, HWND hwnd, IFileOperationProgressSink *pSink)
{
    win16_dialog_primary *p=(win16_dialog_primary *)This;*p->unsupported=TRUE;return IFileSaveDialog_ApplyProperties((IFileSaveDialog *)p->variant ,psi,pStore,hwnd,pSink);
}

static const IFileSaveDialogVtbl save_vtable={
    save_QueryInterface,
    save_AddRef,
    save_Release,
    save_Show,
    save_SetFileTypes,
    save_SetFileTypeIndex,
    save_GetFileTypeIndex,
    save_Advise,
    save_Unadvise,
    save_SetOptions,
    save_GetOptions,
    save_SetDefaultFolder,
    save_SetFolder,
    save_GetFolder,
    save_GetCurrentSelection,
    save_SetFileName,
    save_GetFileName,
    save_SetTitle,
    save_SetOkButtonLabel,
    save_SetFileNameLabel,
    save_GetResult,
    save_AddPlace,
    save_SetDefaultExtension,
    save_Close,
    save_SetClientGuid,
    save_ClearClientData,
    save_SetFilter,
    save_SetSaveAsItem,
    save_SetProperties,
    save_SetCollectedProperties,
    save_GetProperties,
    save_ApplyProperties
};

void win16_dialog_primary_init(win16_dialog_primary *adapter,void *owner,IUnknown *variant,BOOL *unsupported,BOOL save,
    const win16_dialog_customize_services *identity,win16_dialog_v2_call call)
{
    adapter->vtable=save?(const void *)&save_vtable:(const void *)&open_vtable;
    adapter->owner=owner;adapter->variant=variant;adapter->unsupported=unsupported;adapter->identity=identity;adapter->call=call;
}

/* Wrap only interfaces whose complete COM identity we control. */
BOOL win16_dialog_proxy_iid(REFIID iid,BOOL save)
{
    return iid && (IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_IModalWindow)||
        IsEqualIID(iid,&IID_IFileDialog)||IsEqualIID(iid,&IID_IOleWindow)||
        IsEqualIID(iid,&IID_IFileDialog2)||IsEqualIID(iid,&IID_IFileDialogCustomize)||
        (!save && IsEqualIID(iid,&IID_IFileOpenDialog))||(save && IsEqualIID(iid,&IID_IFileSaveDialog)));
}
