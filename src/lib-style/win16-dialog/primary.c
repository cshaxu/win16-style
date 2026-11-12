/* IFileOpenDialog and IFileSaveDialog facades; owner supplies state and identity. */
#define COBJMACROS
#include "primary.h"
#include "common_methods.h"
#define PRIMARY_CALL(p,op,a,b) ((p)->call((p)->owner,V2_##op,(ULONG_PTR)(a),(ULONG_PTR)(b)))

WIN16_DIALOG_COMMON(open,IFileOpenDialog,win16_dialog_primary,(IFileOpenDialog *)p->variant,PRIMARY_GET_SELECTION,PRIMARY_CLOSE,PRIMARY_CALL)
WIN16_DIALOG_COMMON(save,IFileSaveDialog,win16_dialog_primary,(IFileSaveDialog *)p->variant,PRIMARY_GET_SELECTION,PRIMARY_CLOSE,PRIMARY_CALL)

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
