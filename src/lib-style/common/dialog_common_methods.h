#ifndef STYLE_DIALOG_COMMON_METHODS_H
#define STYLE_DIALOG_COMMON_METHODS_H
/* Shared IFileDialog methods have identical behavior, but require each
 * interface's correctly typed COM function pointers. */
#define STYLE_DIALOG_COMMON(PREFIX,IFACE,ADAPTER,NATIVE,SELECTION,CLOSE_OP,CALL) \
static HRESULT STDMETHODCALLTYPE PREFIX##_QueryInterface(IFACE *This, REFIID riid, void **ppvObject) \
{ \
    ADAPTER *p=(ADAPTER *)This;return p->identity->query(p->owner,riid,ppvObject); \
} \
 \
static ULONG STDMETHODCALLTYPE PREFIX##_AddRef(IFACE *This) \
{ \
    ADAPTER *p=(ADAPTER *)This;return p->identity->add(p->owner); \
} \
 \
static ULONG STDMETHODCALLTYPE PREFIX##_Release(IFACE *This) \
{ \
    ADAPTER *p=(ADAPTER *)This;return p->identity->release(p->owner); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_Show(IFACE *This, HWND hwndOwner) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,SHOW,hwndOwner,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetFileTypes(IFACE *This, UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,TYPES,cFileTypes,rgFilterSpec); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetFileTypeIndex(IFACE *This, UINT iFileType) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,SET_INDEX,iFileType,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_GetFileTypeIndex(IFACE *This, UINT *piFileType) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,GET_INDEX,piFileType,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_Advise(IFACE *This, IFileDialogEvents *pfde, DWORD *pdwCookie) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,ADVISE,pfde,pdwCookie); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_Unadvise(IFACE *This, DWORD dwCookie) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,UNADVISE,dwCookie,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetOptions(IFACE *This, FILEOPENDIALOGOPTIONS fos) \
{ \
    ADAPTER *p=(ADAPTER *)This;return IFACE##_SetOptions(NATIVE ,fos); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_GetOptions(IFACE *This, FILEOPENDIALOGOPTIONS *pfos) \
{ \
    ADAPTER *p=(ADAPTER *)This;return IFACE##_GetOptions(NATIVE ,pfos); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetDefaultFolder(IFACE *This, IShellItem *psi) \
{ \
    ADAPTER *p=(ADAPTER *)This;return IFACE##_SetDefaultFolder(NATIVE ,psi); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetFolder(IFACE *This, IShellItem *psi) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,SET_FOLDER,psi,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_GetFolder(IFACE *This, IShellItem **ppsi) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,GET_FOLDER,ppsi,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_GetCurrentSelection(IFACE *This, IShellItem **ppsi) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,SELECTION,ppsi,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetFileName(IFACE *This, LPCWSTR pszName) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,SET_NAME,pszName,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_GetFileName(IFACE *This, LPWSTR *pszName) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,GET_NAME,pszName,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetTitle(IFACE *This, LPCWSTR pszTitle) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,TITLE,pszTitle,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetOkButtonLabel(IFACE *This, LPCWSTR pszText) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,LABEL,pszText,TRUE); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetFileNameLabel(IFACE *This, LPCWSTR pszLabel) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,LABEL,pszLabel,FALSE); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_GetResult(IFACE *This, IShellItem **ppsi) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,RESULT,ppsi,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_AddPlace(IFACE *This, IShellItem *psi, FDAP fdap) \
{ \
    ADAPTER *p=(ADAPTER *)This;*p->unsupported=TRUE;return IFACE##_AddPlace(NATIVE ,psi,fdap); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetDefaultExtension(IFACE *This, LPCWSTR pszDefaultExtension) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,EXTENSION,pszDefaultExtension,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_Close(IFACE *This, HRESULT hr) \
{ \
    ADAPTER *p=(ADAPTER *)This;return CALL(p,CLOSE_OP,hr,0); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetClientGuid(IFACE *This, REFGUID guid) \
{ \
    ADAPTER *p=(ADAPTER *)This;*p->unsupported=TRUE;return IFACE##_SetClientGuid(NATIVE ,guid); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_ClearClientData(IFACE *This) \
{ \
    ADAPTER *p=(ADAPTER *)This;*p->unsupported=TRUE;return IFACE##_ClearClientData(NATIVE); \
} \
 \
static HRESULT STDMETHODCALLTYPE PREFIX##_SetFilter(IFACE *This, IShellItemFilter *pFilter) \
{ \
    ADAPTER *p=(ADAPTER *)This;*p->unsupported=TRUE;return IFACE##_SetFilter(NATIVE ,pFilter); \
}

#endif
