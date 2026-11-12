/* IFileDialog2 facade; owner supplies stateful operations and COM identity. */
#define COBJMACROS
#include "lib-style/common/dialog_v2.h"
#include "lib-style/common/dialog_common_methods.h"
#include "lib-style/common/classic_file_contract.h"
#define STYLE_DIALOG_V2_CALL(p,op,a,b) ((p)->call((p)->owner,STYLE_DIALOG_V2_##op,(ULONG_PTR)(a),(ULONG_PTR)(b)))

STYLE_DIALOG_COMMON(v2,IFileDialog2,style_dialog_v2,p->native,GET_SELECTION,CLOSE,STYLE_DIALOG_V2_CALL)

static HRESULT STDMETHODCALLTYPE v2_SetCancelButtonLabel(IFileDialog2 *This, LPCWSTR pszLabel)
{
    style_dialog_v2 *p=(style_dialog_v2 *)This;
    HRESULT hr=IFileDialog2_SetCancelButtonLabel(p->native,pszLabel);if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(!style_dialog_copy_text(pszLabel,&copy)){*p->unsupported=TRUE;return hr;}
    if(*p->cancel_label)CoTaskMemFree(*p->cancel_label);
    *p->cancel_label=copy;return hr;
}

static HRESULT STDMETHODCALLTYPE v2_SetNavigationRoot(IFileDialog2 *This, IShellItem *psi)
{
    style_dialog_v2 *p=(style_dialog_v2 *)This;*p->unsupported=TRUE;return IFileDialog2_SetNavigationRoot(p->native ,psi);
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

void style_dialog_v2_init(style_dialog_v2 *adapter,void *owner,IFileDialog2 *native,BOOL *unsupported,
    WCHAR **cancel_label,const style_dialog_identity_services *identity,style_dialog_v2_call call)
{
    adapter->iface.lpVtbl=&v2_vtable;adapter->owner=owner;adapter->native=native;
    adapter->unsupported=unsupported;adapter->cancel_label=cancel_label;adapter->identity=identity;adapter->call=call;
}
