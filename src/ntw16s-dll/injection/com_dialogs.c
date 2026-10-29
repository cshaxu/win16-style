/* Per-object adapter for the common file dialog.  The system COM object still
 * validates configuration; simple filesystem requests use the classic dialog. */
#define COBJMACROS
#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <commdlg.h>
#include <dlgs.h>
#include <wchar.h>
#include <string.h>
#include <stddef.h>

struct event_adapter;
typedef struct file_proxy {
    const void *vtable;
    LONG refs;
    BOOL save, unsupported, completed;
    IFileDialog *native;
    IUnknown *variant;
    IOleWindow ole;
    IOleWindow *ole_native;
    HWND active_window;
    IFileDialog2 v2;
    IFileDialog2 *v2_native;
    IFileDialogCustomize custom;
    IFileDialogCustomize *custom_native;
    IShellItem *result;
    IShellItem *live_folder;
    HRESULT live_folder_error;
    IShellItemArray *results;
    WCHAR *title,*filter,*extension,*ok_label,*cancel_label,*file_label;
    WCHAR *pending_folder;
    UINT filter_index;
    struct event_adapter *events;
} file_proxy;
static IOleWindowVtbl ole_vtable;
static IFileDialogCustomizeVtbl custom_vtable;
static IFileDialog2Vtbl v2_vtable;
static const IFileOpenDialogVtbl open_vtable;
static const IFileSaveDialogVtbl save_vtable;
static HRESULT (WINAPI *native_create)(REFCLSID,LPUNKNOWN,DWORD,REFIID,LPVOID *);
static HRESULT (WINAPI *native_create_ex)(REFCLSID,IUnknown *,DWORD,COSERVERINFO *,DWORD,MULTI_QI *);
static HRESULT (WINAPI *native_class_object)(REFCLSID,DWORD,LPVOID,REFIID,LPVOID *);
static _Thread_local unsigned native_creation_depth;
static void proxy_events_release(file_proxy *p);

static ULONG proxy_add(file_proxy *p){return (ULONG)InterlockedIncrement(&p->refs);}
static ULONG proxy_release(file_proxy *p)
{
    LONG refs=InterlockedDecrement(&p->refs);
    if(!refs){proxy_events_release(p);if(p->result)IShellItem_Release(p->result);
        if(p->live_folder)IShellItem_Release(p->live_folder);
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
static HRESULT proxy_query(file_proxy *p,REFIID id,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;
    if(IsEqualIID(id,&IID_IUnknown) || IsEqualIID(id,&IID_IModalWindow) || IsEqualIID(id,&IID_IFileDialog) ||
       (!p->save && IsEqualIID(id,&IID_IFileOpenDialog)) || (p->save && IsEqualIID(id,&IID_IFileSaveDialog))){
        *out=p;proxy_add(p);return S_OK;
    }
    if(IsEqualIID(id,&IID_IOleWindow)){
        if(!p->ole_native){HRESULT hr=IFileDialog_QueryInterface(p->native,id,(void **)&p->ole_native);if(FAILED(hr))return hr;}
        p->ole.lpVtbl=&ole_vtable;*out=&p->ole;proxy_add(p);return S_OK;
    }
    if(IsEqualIID(id,&IID_IFileDialog2)){
        if(!p->v2_native){HRESULT hr=IFileDialog_QueryInterface(p->native,id,(void **)&p->v2_native);if(FAILED(hr))return hr;}
        p->v2.lpVtbl=&v2_vtable;*out=&p->v2;proxy_add(p);return S_OK;
    }
    if(IsEqualIID(id,&IID_IFileDialogCustomize)){
        if(!p->custom_native){HRESULT hr=IFileDialog_QueryInterface(p->native,id,(void **)&p->custom_native);if(FAILED(hr))return hr;}
        p->custom.lpVtbl=&custom_vtable;*out=&p->custom;proxy_add(p);return S_OK;
    }
    p->unsupported=TRUE;
    return IFileDialog_QueryInterface(p->native,id,out);
}
static HRESULT STDMETHODCALLTYPE ole_query(IOleWindow *self,REFIID id,void **out)
{file_proxy *p=(file_proxy *)((BYTE *)self - offsetof(file_proxy,ole));return proxy_query(p,id,out);}
static ULONG STDMETHODCALLTYPE ole_add(IOleWindow *self)
{file_proxy *p=(file_proxy *)((BYTE *)self - offsetof(file_proxy,ole));return proxy_add(p);}
static ULONG STDMETHODCALLTYPE ole_release(IOleWindow *self)
{file_proxy *p=(file_proxy *)((BYTE *)self - offsetof(file_proxy,ole));return proxy_release(p);}
static HRESULT STDMETHODCALLTYPE ole_window(IOleWindow *self,HWND *window)
{
    file_proxy *p=(file_proxy *)((BYTE *)self - offsetof(file_proxy,ole));
    if(!window)return E_POINTER;
    if(p->active_window && IsWindow(p->active_window)){*window=p->active_window;return S_OK;}
    return IOleWindow_GetWindow(p->ole_native,window);
}
static HRESULT STDMETHODCALLTYPE ole_help(IOleWindow *self,BOOL enter)
{file_proxy *p=(file_proxy *)((BYTE *)self - offsetof(file_proxy,ole));return IOleWindow_ContextSensitiveHelp(p->ole_native,enter);}
static IOleWindowVtbl ole_vtable={ole_query,ole_add,ole_release,ole_window,ole_help};
static HRESULT STDMETHODCALLTYPE custom_QueryInterface(IFileDialogCustomize *This, REFIID riid, void **ppvObject)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));return proxy_query(p,riid,ppvObject);
}

static ULONG STDMETHODCALLTYPE custom_AddRef(IFileDialogCustomize *This)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));return proxy_add(p);
}

static ULONG STDMETHODCALLTYPE custom_Release(IFileDialogCustomize *This)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));return proxy_release(p);
}

static HRESULT STDMETHODCALLTYPE custom_EnableOpenDropDown(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_EnableOpenDropDown(p->custom_native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_AddMenu(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddMenu(p->custom_native ,dwIDCtl,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_AddPushButton(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddPushButton(p->custom_native ,dwIDCtl,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_AddComboBox(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddComboBox(p->custom_native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_AddRadioButtonList(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddRadioButtonList(p->custom_native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_AddCheckButton(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel, WINBOOL bChecked)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddCheckButton(p->custom_native ,dwIDCtl,pszLabel,bChecked);
}

static HRESULT STDMETHODCALLTYPE custom_AddEditBox(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddEditBox(p->custom_native ,dwIDCtl,pszText);
}

static HRESULT STDMETHODCALLTYPE custom_AddSeparator(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddSeparator(p->custom_native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_AddText(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddText(p->custom_native ,dwIDCtl,pszText);
}

static HRESULT STDMETHODCALLTYPE custom_SetControlLabel(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_SetControlLabel(p->custom_native ,dwIDCtl,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_GetControlState(IFileDialogCustomize *This, DWORD dwIDCtl, CDCONTROLSTATEF *pdwState)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_GetControlState(p->custom_native ,dwIDCtl,pdwState);
}

static HRESULT STDMETHODCALLTYPE custom_SetControlState(IFileDialogCustomize *This, DWORD dwIDCtl, CDCONTROLSTATEF dwState)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_SetControlState(p->custom_native ,dwIDCtl,dwState);
}

static HRESULT STDMETHODCALLTYPE custom_GetEditBoxText(IFileDialogCustomize *This, DWORD dwIDCtl, WCHAR **ppszText)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_GetEditBoxText(p->custom_native ,dwIDCtl,ppszText);
}

static HRESULT STDMETHODCALLTYPE custom_SetEditBoxText(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_SetEditBoxText(p->custom_native ,dwIDCtl,pszText);
}

static HRESULT STDMETHODCALLTYPE custom_GetCheckButtonState(IFileDialogCustomize *This, DWORD dwIDCtl, WINBOOL *pbChecked)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_GetCheckButtonState(p->custom_native ,dwIDCtl,pbChecked);
}

static HRESULT STDMETHODCALLTYPE custom_SetCheckButtonState(IFileDialogCustomize *This, DWORD dwIDCtl, WINBOOL bChecked)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_SetCheckButtonState(p->custom_native ,dwIDCtl,bChecked);
}

static HRESULT STDMETHODCALLTYPE custom_AddControlItem(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_AddControlItem(p->custom_native ,dwIDCtl,dwIDItem,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_RemoveControlItem(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_RemoveControlItem(p->custom_native ,dwIDCtl,dwIDItem);
}

static HRESULT STDMETHODCALLTYPE custom_RemoveAllControlItems(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_RemoveAllControlItems(p->custom_native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_GetControlItemState(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, CDCONTROLSTATEF *pdwState)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_GetControlItemState(p->custom_native ,dwIDCtl,dwIDItem,pdwState);
}

static HRESULT STDMETHODCALLTYPE custom_SetControlItemState(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, CDCONTROLSTATEF dwState)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_SetControlItemState(p->custom_native ,dwIDCtl,dwIDItem,dwState);
}

static HRESULT STDMETHODCALLTYPE custom_GetSelectedControlItem(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD *pdwIDItem)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_GetSelectedControlItem(p->custom_native ,dwIDCtl,pdwIDItem);
}

static HRESULT STDMETHODCALLTYPE custom_SetSelectedControlItem(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_SetSelectedControlItem(p->custom_native ,dwIDCtl,dwIDItem);
}

static HRESULT STDMETHODCALLTYPE custom_StartVisualGroup(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_StartVisualGroup(p->custom_native ,dwIDCtl,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_EndVisualGroup(IFileDialogCustomize *This)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_EndVisualGroup(p->custom_native);
}

static HRESULT STDMETHODCALLTYPE custom_MakeProminent(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_MakeProminent(p->custom_native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_SetControlItemText(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,custom));p->unsupported=TRUE;return IFileDialogCustomize_SetControlItemText(p->custom_native ,dwIDCtl,dwIDItem,pszLabel);
}

static IFileDialogCustomizeVtbl custom_vtable={
    custom_QueryInterface,
    custom_AddRef,
    custom_Release,
    custom_EnableOpenDropDown,
    custom_AddMenu,
    custom_AddPushButton,
    custom_AddComboBox,
    custom_AddRadioButtonList,
    custom_AddCheckButton,
    custom_AddEditBox,
    custom_AddSeparator,
    custom_AddText,
    custom_SetControlLabel,
    custom_GetControlState,
    custom_SetControlState,
    custom_GetEditBoxText,
    custom_SetEditBoxText,
    custom_GetCheckButtonState,
    custom_SetCheckButtonState,
    custom_AddControlItem,
    custom_RemoveControlItem,
    custom_RemoveAllControlItems,
    custom_GetControlItemState,
    custom_SetControlItemState,
    custom_GetSelectedControlItem,
    custom_SetSelectedControlItem,
    custom_StartVisualGroup,
    custom_EndVisualGroup,
    custom_MakeProminent,
    custom_SetControlItemText
};

/* Native-dialog fallback must report the proxy identity to callbacks. The
 * adapter holds a weak owner: destruction detaches it before releasing native
 * COM references, avoiding a dialog/event reference cycle. */
typedef struct event_adapter {
    IFileDialogEvents iface;LONG refs;file_proxy *owner;IFileDialogEvents *target;
    DWORD cookie;struct event_adapter *next;
} event_adapter;
static HRESULT STDMETHODCALLTYPE event_query(IFileDialogEvents *self,REFIID iid,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;if(!IsEqualIID(iid,&IID_IUnknown) && !IsEqualIID(iid,&IID_IFileDialogEvents))return E_NOINTERFACE;
    *out=self;InterlockedIncrement(&((event_adapter *)self)->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE event_add(IFileDialogEvents *self)
{return (ULONG)InterlockedIncrement(&((event_adapter *)self)->refs);}
static ULONG STDMETHODCALLTYPE event_release(IFileDialogEvents *self)
{
    event_adapter *event=(event_adapter *)self;LONG refs=InterlockedDecrement(&event->refs);
    if(!refs){IFileDialogEvents_Release(event->target);HeapFree(GetProcessHeap(),0,event);}
    return (ULONG)refs;
}
static IFileDialog *event_dialog(event_adapter *event,IFileDialog *native,file_proxy **held)
{
    *held=event->owner;if(*held){proxy_add(*held);return (IFileDialog *)*held;}return native;
}
#define EVENT_SIMPLE(name,method) \
static HRESULT STDMETHODCALLTYPE name(IFileDialogEvents *self,IFileDialog *native) \
{event_adapter *e=(event_adapter *)self;event_add(self);file_proxy *held;IFileDialog *d=event_dialog(e,native,&held); \
 HRESULT hr=IFileDialogEvents_##method(e->target,d);if(held)proxy_release(held);event_release(self);return hr;}
EVENT_SIMPLE(event_ok,OnFileOk)
EVENT_SIMPLE(event_folder,OnFolderChange)
EVENT_SIMPLE(event_selection,OnSelectionChange)
EVENT_SIMPLE(event_type,OnTypeChange)
#undef EVENT_SIMPLE
static HRESULT STDMETHODCALLTYPE event_changing(IFileDialogEvents *self,IFileDialog *native,IShellItem *item)
{
    event_adapter *e=(event_adapter *)self;event_add(self);file_proxy *held;IFileDialog *d=event_dialog(e,native,&held);
    HRESULT hr=IFileDialogEvents_OnFolderChanging(e->target,d,item);if(held)proxy_release(held);event_release(self);return hr;
}
static HRESULT STDMETHODCALLTYPE event_share(IFileDialogEvents *self,IFileDialog *native,IShellItem *item,FDE_SHAREVIOLATION_RESPONSE *response)
{
    event_adapter *e=(event_adapter *)self;event_add(self);file_proxy *held;IFileDialog *d=event_dialog(e,native,&held);
    HRESULT hr=IFileDialogEvents_OnShareViolation(e->target,d,item,response);if(held)proxy_release(held);event_release(self);return hr;
}
static HRESULT STDMETHODCALLTYPE event_overwrite(IFileDialogEvents *self,IFileDialog *native,IShellItem *item,FDE_OVERWRITE_RESPONSE *response)
{
    event_adapter *e=(event_adapter *)self;event_add(self);file_proxy *held;IFileDialog *d=event_dialog(e,native,&held);
    HRESULT hr=IFileDialogEvents_OnOverwrite(e->target,d,item,response);if(held)proxy_release(held);event_release(self);return hr;
}
static IFileDialogEventsVtbl event_vtable={event_query,event_add,event_release,event_ok,event_changing,event_folder,event_selection,event_share,event_type,event_overwrite};
static HRESULT proxy_advise(file_proxy *p,IFileDialogEvents *target,DWORD *cookie)
{
    if(!target||!cookie)return E_INVALIDARG;
    event_adapter *e=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*e));if(!e)return E_OUTOFMEMORY;
    e->iface.lpVtbl=&event_vtable;e->refs=1;e->owner=p;e->target=target;IFileDialogEvents_AddRef(target);
    HRESULT hr=IFileDialog_Advise(p->native,&e->iface,cookie);
    if(FAILED(hr)){e->owner=NULL;event_release(&e->iface);return hr;}
    e->cookie=*cookie;e->next=p->events;p->events=e;return hr;
}
static HRESULT proxy_unadvise(file_proxy *p,DWORD cookie)
{
    HRESULT hr=IFileDialog_Unadvise(p->native,cookie);if(FAILED(hr))return hr;
    event_adapter **link=&p->events;
    while(*link && (*link)->cookie!=cookie)link=&(*link)->next;
    if(*link){event_adapter *e=*link;*link=e->next;e->owner=NULL;event_release(&e->iface);}
    return hr;
}
static void proxy_events_release(file_proxy *p)
{
    while(p->events){event_adapter *e=p->events;p->events=e->next;e->owner=NULL;
        (void)IFileDialog_Unadvise(p->native,e->cookie);event_release(&e->iface);}
}

static HRESULT proxy_result(file_proxy *p,IShellItem **item)
{
    if(!item)return E_POINTER;
    if(!p->completed)return IFileDialog_GetResult(p->native,item);
    if(p->results){*item=NULL;return E_FAIL;}
    *item=p->result;if(!*item)return E_FAIL;IShellItem_AddRef(*item);return S_OK;
}
typedef struct native_call {IFileDialog *dialog;HWND owner;} native_call;
static HRESULT native_show(void *data)
{native_call *call=data;return IFileDialog_Show(call->dialog,call->owner);}
static HRESULT show_native(file_proxy *p,HWND owner)
{native_call call={p->native,owner};return ntw16s_dialog_style_scope(native_show,&call);}
typedef struct folder_call {HWND owner;PCWSTR title,initial,ok_label,cancel_label,file_label;PWSTR result;DWORD capacity;} folder_call;
static HRESULT classic_folder_show(void *data)
{folder_call *call=data;return ntw16s_classic_folder(call->owner,call->title,call->initial,call->ok_label,call->cancel_label,call->file_label,call->result,call->capacity);}
typedef struct classic_call {OPENFILENAMEW *ofn;BOOL save;BOOL selected;DWORD error;} classic_call;
static HRESULT classic_show(void *data)
{
    classic_call *call=data;call->selected=call->save?GetSaveFileNameW(call->ofn):GetOpenFileNameW(call->ofn);
    call->error=call->selected?0:CommDlgExtendedError();return S_OK;
}
/* Shell needs metadata to represent a save target which does not exist yet. */
typedef struct bind_data {IFileSystemBindData iface;LONG refs;WIN32_FIND_DATAW data;} bind_data;
static HRESULT STDMETHODCALLTYPE bind_query(IFileSystemBindData *self,REFIID id,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;
    if(!IsEqualIID(id,&IID_IUnknown) && !IsEqualIID(id,&IID_IFileSystemBindData))return E_NOINTERFACE;
    *out=self;InterlockedIncrement(&((bind_data *)self)->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE bind_add(IFileSystemBindData *self)
{return (ULONG)InterlockedIncrement(&((bind_data *)self)->refs);}
static ULONG STDMETHODCALLTYPE bind_release(IFileSystemBindData *self)
{LONG refs=InterlockedDecrement(&((bind_data *)self)->refs);if(!refs)HeapFree(GetProcessHeap(),0,self);return (ULONG)refs;}
static HRESULT STDMETHODCALLTYPE bind_set(IFileSystemBindData *self,const WIN32_FIND_DATAW *data)
{if(!data)return E_POINTER;((bind_data *)self)->data=*data;return S_OK;}
static HRESULT STDMETHODCALLTYPE bind_get(IFileSystemBindData *self,WIN32_FIND_DATAW *data)
{if(!data)return E_POINTER;*data=((bind_data *)self)->data;return S_OK;}
static IFileSystemBindDataVtbl bind_vtable={bind_query,bind_add,bind_release,bind_set,bind_get};
static HRESULT save_item(PCWSTR path,IShellItem **item)
{
    bind_data *data=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*data));if(!data)return E_OUTOFMEMORY;
    data->iface.lpVtbl=&bind_vtable;data->refs=1;data->data.dwFileAttributes=FILE_ATTRIBUTE_NORMAL;
    IBindCtx *ctx=NULL;HRESULT hr=CreateBindCtx(0,&ctx);
    if(SUCCEEDED(hr))hr=IBindCtx_RegisterObjectParam(ctx,L"File System Bind Data",(IUnknown *)&data->iface);
    if(SUCCEEDED(hr))hr=SHCreateItemFromParsingName(path,ctx,&IID_IShellItem,(void **)item);
    if(ctx)IBindCtx_Release(ctx);
    IFileSystemBindData_Release(&data->iface);return hr;
}
/* Old common dialogs notify after changing their process current directory.
 * Capture that transition immediately instead of consulting the unrelated
 * dormant modern object's initial folder on later COM calls. */
static void classic_capture_folder(file_proxy *p)
{
    DWORD needed=GetCurrentDirectoryW(0,NULL);IShellItem *item=NULL;
    HRESULT hr=needed?S_OK:HRESULT_FROM_WIN32(GetLastError());
    WCHAR *path=needed?CoTaskMemAlloc((size_t)needed*sizeof(WCHAR)):NULL;
    if(needed && !path)hr=E_OUTOFMEMORY;
    if(SUCCEEDED(hr)){
        DWORD copied=GetCurrentDirectoryW(needed,path);
        if(!copied || copied>=needed)hr=HRESULT_FROM_WIN32(copied?ERROR_INSUFFICIENT_BUFFER:GetLastError());
        else hr=SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&item);
    }
    if(path)CoTaskMemFree(path);
    if(p->live_folder)IShellItem_Release(p->live_folder);
    p->live_folder=item;p->live_folder_error=hr;
}
static HRESULT proxy_get_folder(file_proxy *p,IShellItem **item)
{
    if(!p->active_window)return IFileDialog_GetFolder(p->native,item);
    if(!item)return E_POINTER;
    *item=NULL;if(FAILED(p->live_folder_error))return p->live_folder_error;
    if(!p->live_folder)return E_FAIL;
    *item=p->live_folder;IShellItem_AddRef(*item);return S_OK;
}
static const WCHAR folder_message_name[]=L"Win16Style.T20.ComSetFolder";
static HRESULT proxy_set_folder(file_proxy *p,IShellItem *item)
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
    UINT message=RegisterWindowMessageW(folder_message_name);
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
static void classic_navigate_folder(HWND w,file_proxy *p,PCWSTR key)
{
    WCHAR *folder=p->pending_folder;p->pending_folder=NULL;if(!folder)return;
    HWND edit=GetDlgItem(w,edt1);int length=edit?GetWindowTextLengthW(edit):-1;
    WCHAR *filename=length>=0 && length<32768?CoTaskMemAlloc(((size_t)length+1)*sizeof(WCHAR)):NULL;
    if(filename){
        GetWindowTextW(edit,filename,length+1);
        DWORD attrs=GetFileAttributesW(folder);
        if(attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY) && SetWindowTextW(edit,folder)){
            SendMessageW(w,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(w,IDOK));
            if(IsWindow(w) && GetPropW(w,key)==p && IsWindow(edit))SetWindowTextW(edit,filename);
        }
        CoTaskMemFree(filename);
    }
    CoTaskMemFree(folder);
}
static UINT_PTR CALLBACK classic_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    (void)wp;
    static const WCHAR key[]=L"Win16Style.T20.ComDialog";
    if(message==WM_INITDIALOG && lp){OPENFILENAMEW *ofn=(OPENFILENAMEW *)lp;file_proxy *p=(file_proxy *)ofn->lCustData;
        if(p && SetPropW(w,key,p)){p->active_window=w;classic_capture_folder(p);}
        if(p && p->ok_label)SetDlgItemTextW(w,IDOK,p->ok_label);
        if(p && p->cancel_label)SetDlgItemTextW(w,IDCANCEL,p->cancel_label);
        if(p && p->file_label)SetDlgItemTextW(w,stc3,p->file_label);
    }else if(message==WM_NCDESTROY){file_proxy *p=GetPropW(w,key);
        if(p){RemovePropW(w,key);if(p->active_window==w)p->active_window=NULL;
            if(p->live_folder){IShellItem_Release(p->live_folder);p->live_folder=NULL;}
            if(p->pending_folder){CoTaskMemFree(p->pending_folder);p->pending_folder=NULL;}}}
    else if(message>=0xc000 && message<=0xffff){UINT selection=RegisterWindowMessageW(LBSELCHSTRINGW);
        if(selection && message==selection && (wp==lst2 || wp==cmb2)){
            file_proxy *p=GetPropW(w,key);if(p)classic_capture_folder(p);}
        UINT navigate=RegisterWindowMessageW(folder_message_name);
        if(navigate && message==navigate){file_proxy *p=GetPropW(w,key);if(p)classic_navigate_folder(w,p,key);return 1;}}
    return 0;
}
/* The legacy multi-select contract returns space-separated 8.3 names. The
 * file offset separates the directory from its first name even for one file. */
static HRESULT link_target_id(PCWSTR path,PIDLIST_ABSOLUTE *target)
{
    *target=NULL;const WCHAR *extension=wcsrchr(path,L'.');
    if(!extension || lstrcmpiW(extension,L".lnk"))return S_FALSE;
    IShellLinkW *link=NULL;IPersistFile *persist=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&link);
    if(SUCCEEDED(hr))hr=IShellLinkW_QueryInterface(link,&IID_IPersistFile,(void **)&persist);
    if(SUCCEEDED(hr))hr=IPersistFile_Load(persist,path,STGM_READ);
    if(SUCCEEDED(hr))hr=IShellLinkW_GetIDList(link,target);
    if(persist)IPersistFile_Release(persist);
    if(link)IShellLinkW_Release(link);
    if(FAILED(hr) && *target){CoTaskMemFree(*target);*target=NULL;}
    if(SUCCEEDED(hr) && !*target)return E_FAIL;
    return hr;
}
static HRESULT open_item(PCWSTR path,BOOL keep_link,IShellItem **item)
{
    PIDLIST_ABSOLUTE target=NULL;
    if(!keep_link && link_target_id(path,&target)==S_OK){
        HRESULT hr=SHCreateItemFromIDList(target,&IID_IShellItem,(void **)item);
        CoTaskMemFree(target);return hr;
    }
    return SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)item);
}
static HRESULT proxy_multi_result(file_proxy *p,WCHAR *file,WORD offset,BOOL keep_link)
{
    size_t length=wcslen(file);
    if(offset==0 || offset>length || file[offset-1]!=L' '){
        HRESULT hr=open_item(file,keep_link,&p->result);
        if(SUCCEEDED(hr))p->completed=TRUE;
        return hr;
    }
    file[offset-1]=0;WCHAR *token=file+offset;PIDLIST_ABSOLUTE ids[512]={0};UINT count=0;HRESULT hr=S_OK;
    while(*token){
        while(*token==L' ')++token;
        if(!*token)break;
        WCHAR *end=wcschr(token,L' ');if(end)*end=0;
        if(count==ARRAYSIZE(ids)){hr=HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);break;}
        WCHAR path[32768];size_t dirlen=wcslen(file);int written=swprintf(path,ARRAYSIZE(path),L"%ls%ls%ls",file,
            dirlen && file[dirlen-1]==L'\\'?L"":L"\\",token);
        if(written<0 || (size_t)written>=ARRAYSIZE(path)){hr=HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);break;}
        WCHAR normalized[32768];DWORD n=GetLongPathNameW(path,normalized,ARRAYSIZE(normalized));
        PCWSTR selected=n && n<ARRAYSIZE(normalized)?normalized:path;
        hr=keep_link?S_FALSE:link_target_id(selected,&ids[count]);
        if(hr!=S_OK)hr=SHParseDisplayName(selected,NULL,&ids[count],0,NULL);
        if(FAILED(hr))break;
        ++count;if(!end)break;token=end+1;
    }
    if(SUCCEEDED(hr) && count)hr=SHCreateShellItemArrayFromIDLists(count,(PCIDLIST_ABSOLUTE_ARRAY)ids,&p->results);
    else if(SUCCEEDED(hr))hr=E_FAIL;
    for(UINT i=0;i<count;++i)CoTaskMemFree(ids[i]);
    if(SUCCEEDED(hr))p->completed=TRUE;
    return hr;
}
static HRESULT proxy_show(file_proxy *p,HWND owner)
{
    FILEOPENDIALOGOPTIONS flags=0;
    if(p->result){IShellItem_Release(p->result);p->result=NULL;}
    if(p->results){IShellItemArray_Release(p->results);p->results=NULL;}p->completed=FALSE;
    HRESULT hr=IFileDialog_GetOptions(p->native,&flags);
    const DWORD supported=FOS_OVERWRITEPROMPT|FOS_NOCHANGEDIR|FOS_FORCEFILESYSTEM|
        FOS_PATHMUSTEXIST|FOS_FILEMUSTEXIST|FOS_CREATEPROMPT|FOS_NOREADONLYRETURN|FOS_NOTESTFILECREATE|FOS_PICKFOLDERS|FOS_ALLOWMULTISELECT|FOS_FORCESHOWHIDDEN|FOS_NODEREFERENCELINKS;
    if(FAILED(hr)||p->unsupported || p->events || (flags & ~supported))return show_native(p,owner);
    if((flags&FOS_ALLOWMULTISELECT) && (p->save || (flags&FOS_PICKFOLDERS) || !(flags&FOS_FILEMUSTEXIST)))return show_native(p,owner);
    if((flags&FOS_NODEREFERENCELINKS) && (p->save || (flags&FOS_PICKFOLDERS)))return show_native(p,owner);
    if(flags&FOS_FORCESHOWHIDDEN)return show_native(p,owner);
    WCHAR file[32768]=L"";PWSTR value=NULL;IShellItem *folder=NULL;PWSTR folder_path=NULL;
    if(SUCCEEDED(IFileDialog_GetFileName(p->native,&value)) && value){
        if(wcslen(value)>=ARRAYSIZE(file)){CoTaskMemFree(value);return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);}
        wcscpy(file,value);CoTaskMemFree(value);
    }
    if(SUCCEEDED(IFileDialog_GetFolder(p->native,&folder)) && folder){
        (void)IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&folder_path);IShellItem_Release(folder);
    }
    if(flags&FOS_PICKFOLDERS){
        if(p->save){if(folder_path)CoTaskMemFree(folder_path);return show_native(p,owner);}
        WCHAR selected[32768];folder_call call={owner,p->title,folder_path,p->ok_label,p->cancel_label,p->file_label,selected,ARRAYSIZE(selected)};
        hr=ntw16s_dialog_style_scope(classic_folder_show,&call);if(folder_path)CoTaskMemFree(folder_path);
        if(FAILED(hr))return hr;
        hr=SHCreateItemFromParsingName(selected,NULL,&IID_IShellItem,(void **)&p->result);
        if(SUCCEEDED(hr))p->completed=TRUE;
        return hr;
    }
    OPENFILENAMEW ofn={0};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=owner;
    ofn.lpstrFile=file;ofn.nMaxFile=ARRAYSIZE(file);ofn.lpstrInitialDir=folder_path;ofn.lpstrTitle=p->title;
    ofn.lpstrFilter=p->filter?p->filter:L"All files\0*.*\0\0";ofn.nFilterIndex=p->filter_index?p->filter_index:1;ofn.lpstrDefExt=p->extension;
    ofn.Flags=OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_ENABLEHOOK;ofn.lpfnHook=classic_hook;ofn.lCustData=(LPARAM)p;
    if(flags&FOS_ALLOWMULTISELECT)ofn.Flags|=OFN_ALLOWMULTISELECT;
    if(flags&FOS_OVERWRITEPROMPT)ofn.Flags|=OFN_OVERWRITEPROMPT;
    if(flags&FOS_NOCHANGEDIR)ofn.Flags|=OFN_NOCHANGEDIR;
    if(flags&FOS_PATHMUSTEXIST)ofn.Flags|=OFN_PATHMUSTEXIST;
    if(flags&FOS_FILEMUSTEXIST)ofn.Flags|=OFN_FILEMUSTEXIST;
    if(flags&FOS_CREATEPROMPT)ofn.Flags|=OFN_CREATEPROMPT;
    if(flags&FOS_NOREADONLYRETURN)ofn.Flags|=OFN_NOREADONLYRETURN;
    if(flags&FOS_NOTESTFILECREATE)ofn.Flags|=OFN_NOTESTFILECREATE;
    if(flags&FOS_NODEREFERENCELINKS)ofn.Flags|=OFN_NODEREFERENCELINKS;
    classic_call call={&ofn,p->save,FALSE,0};
    hr=ntw16s_dialog_style_scope(classic_show,&call);if(folder_path)CoTaskMemFree(folder_path);
    if(FAILED(hr))return hr;
    if(!call.selected)return call.error?HRESULT_FROM_WIN32(ERROR_INVALID_DATA):HRESULT_FROM_WIN32(ERROR_CANCELLED);
    if(p->filter && ofn.nFilterIndex){p->filter_index=ofn.nFilterIndex;(void)IFileDialog_SetFileTypeIndex(p->native,p->filter_index);}
    if(flags&FOS_ALLOWMULTISELECT)return proxy_multi_result(p,file,ofn.nFileOffset,(flags&FOS_NODEREFERENCELINKS)!=0);
    hr=p->save?SHCreateItemFromParsingName(file,NULL,&IID_IShellItem,(void **)&p->result):open_item(file,(flags&FOS_NODEREFERENCELINKS)!=0,&p->result);
    if(p->save && hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))hr=save_item(file,&p->result);
    if(SUCCEEDED(hr))p->completed=TRUE;
    return hr;
}
static HRESULT proxy_title(file_proxy *p,LPCWSTR text)
{
    HRESULT hr=IFileDialog_SetTitle(p->native,text);if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(text){size_t size=(wcslen(text)+1)*sizeof(WCHAR);copy=CoTaskMemAlloc(size);if(!copy){p->unsupported=TRUE;return E_OUTOFMEMORY;}memcpy(copy,text,size);}
    if(p->title)CoTaskMemFree(p->title);
    p->title=copy;return hr;
}
/* Capture only data accepted by the native object; allocation failure leaves
 * its complete state available for a native-dialog fallback. */
static HRESULT proxy_label(file_proxy *p,LPCWSTR text,BOOL ok_button)
{
    HRESULT hr=ok_button?IFileDialog_SetOkButtonLabel(p->native,text):IFileDialog_SetFileNameLabel(p->native,text);
    if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(text){size_t n=wcslen(text)+1;copy=CoTaskMemAlloc(n*sizeof(WCHAR));
        if(!copy){p->unsupported=TRUE;return hr;}memcpy(copy,text,n*sizeof(WCHAR));}
    WCHAR **slot=ok_button?&p->ok_label:&p->file_label;if(*slot)CoTaskMemFree(*slot);*slot=copy;return hr;
}
static HRESULT proxy_types(file_proxy *p,UINT count,const COMDLG_FILTERSPEC *items)
{
    HRESULT hr=IFileDialog_SetFileTypes(p->native,count,items);if(FAILED(hr))return hr;
    size_t chars=1;
    for(UINT i=0;i<count;++i){
        if(!items[i].pszName || !items[i].pszSpec){p->unsupported=TRUE;return hr;}
        size_t name=wcslen(items[i].pszName),spec=wcslen(items[i].pszSpec);
        if(name>32767 || spec>32767 || name+spec+2>65535-chars){p->unsupported=TRUE;return hr;}
        chars+=name+spec+2;
    }
    WCHAR *filter=CoTaskMemAlloc(chars*sizeof(WCHAR));
    if(!filter){p->unsupported=TRUE;return hr;}
    WCHAR *at=filter;
    for(UINT i=0;i<count;++i){size_t n=wcslen(items[i].pszName)+1;memcpy(at,items[i].pszName,n*sizeof(WCHAR));at+=n;
        n=wcslen(items[i].pszSpec)+1;memcpy(at,items[i].pszSpec,n*sizeof(WCHAR));at+=n;}
    *at=0;if(p->filter)CoTaskMemFree(p->filter);p->filter=filter;p->filter_index=1;return hr;
}
static HRESULT proxy_filter_index(file_proxy *p,UINT index)
{
    HRESULT hr=IFileDialog_SetFileTypeIndex(p->native,index);if(SUCCEEDED(hr))p->filter_index=index;return hr;
}
static HRESULT proxy_get_filter_index(file_proxy *p,UINT *index)
{
    if(!p->active_window || !p->filter || !index)return IFileDialog_GetFileTypeIndex(p->native,index);
    HWND combo=GetDlgItem(p->active_window,cmb1);
    if(!combo)return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    LRESULT selected=SendMessageW(combo,CB_GETCURSEL,0,0);
    if(selected==CB_ERR)return E_FAIL;
    *index=(UINT)selected+1;return S_OK;
}
static HRESULT proxy_extension(file_proxy *p,LPCWSTR value)
{
    HRESULT hr=IFileDialog_SetDefaultExtension(p->native,value);if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(value){size_t chars=wcslen(value)+1;copy=CoTaskMemAlloc(chars*sizeof(WCHAR));
        if(!copy){p->unsupported=TRUE;return hr;}memcpy(copy,value,chars*sizeof(WCHAR));}
    if(p->extension)CoTaskMemFree(p->extension);
    p->extension=copy;return hr;
}
/* The native COM object is not showing when a classic file dialog is active.
 * Keep its saved configuration in sync, and use the real classic edit control
 * for live values changed by either the caller or the user. */
static HRESULT proxy_set_filename(file_proxy *p,LPCWSTR name)
{
    HRESULT hr=IFileDialog_SetFileName(p->native,name);
    if(FAILED(hr) || !p->active_window)return hr;
    HWND edit=GetDlgItem(p->active_window,edt1);
    if(!edit || !SetWindowTextW(edit,name))return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    return hr;
}
static HRESULT proxy_get_filename(file_proxy *p,LPWSTR *name)
{
    if(!p->active_window)return IFileDialog_GetFileName(p->native,name);
    if(!name)return E_POINTER;
    *name=NULL;HWND edit=GetDlgItem(p->active_window,edt1);
    if(!edit)return HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE);
    int length=GetWindowTextLengthW(edit);
    if(length<0 || length>=32768)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    WCHAR *value=CoTaskMemAlloc(((size_t)length+1)*sizeof(WCHAR));
    if(!value)return E_OUTOFMEMORY;
    GetWindowTextW(edit,value,length+1);*name=value;return S_OK;
}
static HRESULT STDMETHODCALLTYPE v2_QueryInterface(IFileDialog2 *This, REFIID riid, void **ppvObject)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_query(p,riid,ppvObject);
}

static ULONG STDMETHODCALLTYPE v2_AddRef(IFileDialog2 *This)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_add(p);
}

static ULONG STDMETHODCALLTYPE v2_Release(IFileDialog2 *This)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_release(p);
}

static HRESULT STDMETHODCALLTYPE v2_Show(IFileDialog2 *This, HWND hwndOwner)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_show(p,hwndOwner);
}

static HRESULT STDMETHODCALLTYPE v2_SetFileTypes(IFileDialog2 *This, UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_types(p,cFileTypes,rgFilterSpec);
}

static HRESULT STDMETHODCALLTYPE v2_SetFileTypeIndex(IFileDialog2 *This, UINT iFileType)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_filter_index(p,iFileType);
}

static HRESULT STDMETHODCALLTYPE v2_GetFileTypeIndex(IFileDialog2 *This, UINT *piFileType)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_get_filter_index(p,piFileType);
}

static HRESULT STDMETHODCALLTYPE v2_Advise(IFileDialog2 *This, IFileDialogEvents *pfde, DWORD *pdwCookie)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_advise(p,pfde,pdwCookie);
}

static HRESULT STDMETHODCALLTYPE v2_Unadvise(IFileDialog2 *This, DWORD dwCookie)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_unadvise(p,dwCookie);
}

static HRESULT STDMETHODCALLTYPE v2_SetOptions(IFileDialog2 *This, FILEOPENDIALOGOPTIONS fos)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return IFileDialog2_SetOptions(p->v2_native ,fos);
}

static HRESULT STDMETHODCALLTYPE v2_GetOptions(IFileDialog2 *This, FILEOPENDIALOGOPTIONS *pfos)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return IFileDialog2_GetOptions(p->v2_native ,pfos);
}

static HRESULT STDMETHODCALLTYPE v2_SetDefaultFolder(IFileDialog2 *This, IShellItem *psi)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return IFileDialog2_SetDefaultFolder(p->v2_native ,psi);
}

static HRESULT STDMETHODCALLTYPE v2_SetFolder(IFileDialog2 *This, IShellItem *psi)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_set_folder(p,psi);
}

static HRESULT STDMETHODCALLTYPE v2_GetFolder(IFileDialog2 *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_get_folder(p,ppsi);
}

static HRESULT STDMETHODCALLTYPE v2_GetCurrentSelection(IFileDialog2 *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return IFileDialog2_GetCurrentSelection(p->v2_native ,ppsi);
}

static HRESULT STDMETHODCALLTYPE v2_SetFileName(IFileDialog2 *This, LPCWSTR pszName)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_set_filename(p,pszName);
}

static HRESULT STDMETHODCALLTYPE v2_GetFileName(IFileDialog2 *This, LPWSTR *pszName)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_get_filename(p,pszName);
}

static HRESULT STDMETHODCALLTYPE v2_SetTitle(IFileDialog2 *This, LPCWSTR pszTitle)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_title(p,pszTitle);
}

static HRESULT STDMETHODCALLTYPE v2_SetOkButtonLabel(IFileDialog2 *This, LPCWSTR pszText)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_label(p,pszText,TRUE);
}

static HRESULT STDMETHODCALLTYPE v2_SetFileNameLabel(IFileDialog2 *This, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_label(p,pszLabel,FALSE);
}

static HRESULT STDMETHODCALLTYPE v2_GetResult(IFileDialog2 *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_result(p,ppsi);
}

static HRESULT STDMETHODCALLTYPE v2_AddPlace(IFileDialog2 *This, IShellItem *psi, FDAP fdap)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));p->unsupported=TRUE;return IFileDialog2_AddPlace(p->v2_native ,psi,fdap);
}

static HRESULT STDMETHODCALLTYPE v2_SetDefaultExtension(IFileDialog2 *This, LPCWSTR pszDefaultExtension)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));return proxy_extension(p,pszDefaultExtension);
}

static HRESULT STDMETHODCALLTYPE v2_Close(IFileDialog2 *This, HRESULT hr)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));p->unsupported=TRUE;return IFileDialog2_Close(p->v2_native ,hr);
}

static HRESULT STDMETHODCALLTYPE v2_SetClientGuid(IFileDialog2 *This, REFGUID guid)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));p->unsupported=TRUE;return IFileDialog2_SetClientGuid(p->v2_native ,guid);
}

static HRESULT STDMETHODCALLTYPE v2_ClearClientData(IFileDialog2 *This)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));p->unsupported=TRUE;return IFileDialog2_ClearClientData(p->v2_native);
}

static HRESULT STDMETHODCALLTYPE v2_SetFilter(IFileDialog2 *This, IShellItemFilter *pFilter)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));p->unsupported=TRUE;return IFileDialog2_SetFilter(p->v2_native ,pFilter);
}

static HRESULT STDMETHODCALLTYPE v2_SetCancelButtonLabel(IFileDialog2 *This, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));
    HRESULT hr=IFileDialog2_SetCancelButtonLabel(p->v2_native,pszLabel);if(FAILED(hr))return hr;
    WCHAR *copy=NULL;
    if(pszLabel){size_t n=wcslen(pszLabel)+1;copy=CoTaskMemAlloc(n*sizeof(WCHAR));
        if(!copy){p->unsupported=TRUE;return hr;}memcpy(copy,pszLabel,n*sizeof(WCHAR));}
    if(p->cancel_label)CoTaskMemFree(p->cancel_label);
    p->cancel_label=copy;return hr;
}

static HRESULT STDMETHODCALLTYPE v2_SetNavigationRoot(IFileDialog2 *This, IShellItem *psi)
{
    file_proxy *p=(file_proxy *)((BYTE *)This - offsetof(file_proxy,v2));p->unsupported=TRUE;return IFileDialog2_SetNavigationRoot(p->v2_native ,psi);
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

static HRESULT STDMETHODCALLTYPE open_QueryInterface(IFileOpenDialog *This, REFIID riid, void **ppvObject)
{
    file_proxy *p=(file_proxy *)This;return proxy_query(p,riid,ppvObject);
}

static ULONG STDMETHODCALLTYPE open_AddRef(IFileOpenDialog *This)
{
    file_proxy *p=(file_proxy *)This;return proxy_add(p);
}

static ULONG STDMETHODCALLTYPE open_Release(IFileOpenDialog *This)
{
    file_proxy *p=(file_proxy *)This;return proxy_release(p);
}

static HRESULT STDMETHODCALLTYPE open_Show(IFileOpenDialog *This, HWND hwndOwner)
{
    file_proxy *p=(file_proxy *)This;return proxy_show(p,hwndOwner);
}

static HRESULT STDMETHODCALLTYPE open_SetFileTypes(IFileOpenDialog *This, UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec)
{
    file_proxy *p=(file_proxy *)This;return proxy_types(p,cFileTypes,rgFilterSpec);
}

static HRESULT STDMETHODCALLTYPE open_SetFileTypeIndex(IFileOpenDialog *This, UINT iFileType)
{
    file_proxy *p=(file_proxy *)This;return proxy_filter_index(p,iFileType);
}

static HRESULT STDMETHODCALLTYPE open_GetFileTypeIndex(IFileOpenDialog *This, UINT *piFileType)
{
    file_proxy *p=(file_proxy *)This;return proxy_get_filter_index(p,piFileType);
}

static HRESULT STDMETHODCALLTYPE open_Advise(IFileOpenDialog *This, IFileDialogEvents *pfde, DWORD *pdwCookie)
{
    file_proxy *p=(file_proxy *)This;return proxy_advise(p,pfde,pdwCookie);
}

static HRESULT STDMETHODCALLTYPE open_Unadvise(IFileOpenDialog *This, DWORD dwCookie)
{
    file_proxy *p=(file_proxy *)This;return proxy_unadvise(p,dwCookie);
}

static HRESULT STDMETHODCALLTYPE open_SetOptions(IFileOpenDialog *This, FILEOPENDIALOGOPTIONS fos)
{
    file_proxy *p=(file_proxy *)This;return IFileOpenDialog_SetOptions((IFileOpenDialog *)p->variant ,fos);
}

static HRESULT STDMETHODCALLTYPE open_GetOptions(IFileOpenDialog *This, FILEOPENDIALOGOPTIONS *pfos)
{
    file_proxy *p=(file_proxy *)This;return IFileOpenDialog_GetOptions((IFileOpenDialog *)p->variant ,pfos);
}

static HRESULT STDMETHODCALLTYPE open_SetDefaultFolder(IFileOpenDialog *This, IShellItem *psi)
{
    file_proxy *p=(file_proxy *)This;return IFileOpenDialog_SetDefaultFolder((IFileOpenDialog *)p->variant ,psi);
}

static HRESULT STDMETHODCALLTYPE open_SetFolder(IFileOpenDialog *This, IShellItem *psi)
{
    file_proxy *p=(file_proxy *)This;return proxy_set_folder(p,psi);
}

static HRESULT STDMETHODCALLTYPE open_GetFolder(IFileOpenDialog *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)This;return proxy_get_folder(p,ppsi);
}

static HRESULT STDMETHODCALLTYPE open_GetCurrentSelection(IFileOpenDialog *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)This;return IFileOpenDialog_GetCurrentSelection((IFileOpenDialog *)p->variant ,ppsi);
}

static HRESULT STDMETHODCALLTYPE open_SetFileName(IFileOpenDialog *This, LPCWSTR pszName)
{
    file_proxy *p=(file_proxy *)This;return proxy_set_filename(p,pszName);
}

static HRESULT STDMETHODCALLTYPE open_GetFileName(IFileOpenDialog *This, LPWSTR *pszName)
{
    file_proxy *p=(file_proxy *)This;return proxy_get_filename(p,pszName);
}

static HRESULT STDMETHODCALLTYPE open_SetTitle(IFileOpenDialog *This, LPCWSTR pszTitle)
{
    file_proxy *p=(file_proxy *)This;return proxy_title(p,pszTitle);
}

static HRESULT STDMETHODCALLTYPE open_SetOkButtonLabel(IFileOpenDialog *This, LPCWSTR pszText)
{
    file_proxy *p=(file_proxy *)This;return proxy_label(p,pszText,TRUE);
}

static HRESULT STDMETHODCALLTYPE open_SetFileNameLabel(IFileOpenDialog *This, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)This;return proxy_label(p,pszLabel,FALSE);
}

static HRESULT STDMETHODCALLTYPE open_GetResult(IFileOpenDialog *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)This;return proxy_result(p,ppsi);
}

static HRESULT STDMETHODCALLTYPE open_AddPlace(IFileOpenDialog *This, IShellItem *psi, FDAP fdap)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileOpenDialog_AddPlace((IFileOpenDialog *)p->variant ,psi,fdap);
}

static HRESULT STDMETHODCALLTYPE open_SetDefaultExtension(IFileOpenDialog *This, LPCWSTR pszDefaultExtension)
{
    file_proxy *p=(file_proxy *)This;return proxy_extension(p,pszDefaultExtension);
}

static HRESULT STDMETHODCALLTYPE open_Close(IFileOpenDialog *This, HRESULT hr)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileOpenDialog_Close((IFileOpenDialog *)p->variant ,hr);
}

static HRESULT STDMETHODCALLTYPE open_SetClientGuid(IFileOpenDialog *This, REFGUID guid)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileOpenDialog_SetClientGuid((IFileOpenDialog *)p->variant ,guid);
}

static HRESULT STDMETHODCALLTYPE open_ClearClientData(IFileOpenDialog *This)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileOpenDialog_ClearClientData((IFileOpenDialog *)p->variant);
}

static HRESULT STDMETHODCALLTYPE open_SetFilter(IFileOpenDialog *This, IShellItemFilter *pFilter)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileOpenDialog_SetFilter((IFileOpenDialog *)p->variant ,pFilter);
}

static HRESULT STDMETHODCALLTYPE open_GetResults(IFileOpenDialog *This, IShellItemArray **ppenum)
{
    file_proxy *p=(file_proxy *)This;if(p->completed && p->results){if(!ppenum)return E_POINTER;*ppenum=p->results;IShellItemArray_AddRef(*ppenum);return S_OK;}if(p->completed && p->result)return SHCreateShellItemArrayFromShellItem(p->result,&IID_IShellItemArray,(void **)ppenum);return IFileOpenDialog_GetResults((IFileOpenDialog *)p->variant,ppenum);
}

static HRESULT STDMETHODCALLTYPE open_GetSelectedItems(IFileOpenDialog *This, IShellItemArray **ppsai)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileOpenDialog_GetSelectedItems((IFileOpenDialog *)p->variant ,ppsai);
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
    file_proxy *p=(file_proxy *)This;return proxy_query(p,riid,ppvObject);
}

static ULONG STDMETHODCALLTYPE save_AddRef(IFileSaveDialog *This)
{
    file_proxy *p=(file_proxy *)This;return proxy_add(p);
}

static ULONG STDMETHODCALLTYPE save_Release(IFileSaveDialog *This)
{
    file_proxy *p=(file_proxy *)This;return proxy_release(p);
}

static HRESULT STDMETHODCALLTYPE save_Show(IFileSaveDialog *This, HWND hwndOwner)
{
    file_proxy *p=(file_proxy *)This;return proxy_show(p,hwndOwner);
}

static HRESULT STDMETHODCALLTYPE save_SetFileTypes(IFileSaveDialog *This, UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec)
{
    file_proxy *p=(file_proxy *)This;return proxy_types(p,cFileTypes,rgFilterSpec);
}

static HRESULT STDMETHODCALLTYPE save_SetFileTypeIndex(IFileSaveDialog *This, UINT iFileType)
{
    file_proxy *p=(file_proxy *)This;return proxy_filter_index(p,iFileType);
}

static HRESULT STDMETHODCALLTYPE save_GetFileTypeIndex(IFileSaveDialog *This, UINT *piFileType)
{
    file_proxy *p=(file_proxy *)This;return proxy_get_filter_index(p,piFileType);
}

static HRESULT STDMETHODCALLTYPE save_Advise(IFileSaveDialog *This, IFileDialogEvents *pfde, DWORD *pdwCookie)
{
    file_proxy *p=(file_proxy *)This;return proxy_advise(p,pfde,pdwCookie);
}

static HRESULT STDMETHODCALLTYPE save_Unadvise(IFileSaveDialog *This, DWORD dwCookie)
{
    file_proxy *p=(file_proxy *)This;return proxy_unadvise(p,dwCookie);
}

static HRESULT STDMETHODCALLTYPE save_SetOptions(IFileSaveDialog *This, FILEOPENDIALOGOPTIONS fos)
{
    file_proxy *p=(file_proxy *)This;return IFileSaveDialog_SetOptions((IFileSaveDialog *)p->variant ,fos);
}

static HRESULT STDMETHODCALLTYPE save_GetOptions(IFileSaveDialog *This, FILEOPENDIALOGOPTIONS *pfos)
{
    file_proxy *p=(file_proxy *)This;return IFileSaveDialog_GetOptions((IFileSaveDialog *)p->variant ,pfos);
}

static HRESULT STDMETHODCALLTYPE save_SetDefaultFolder(IFileSaveDialog *This, IShellItem *psi)
{
    file_proxy *p=(file_proxy *)This;return IFileSaveDialog_SetDefaultFolder((IFileSaveDialog *)p->variant ,psi);
}

static HRESULT STDMETHODCALLTYPE save_SetFolder(IFileSaveDialog *This, IShellItem *psi)
{
    file_proxy *p=(file_proxy *)This;return proxy_set_folder(p,psi);
}

static HRESULT STDMETHODCALLTYPE save_GetFolder(IFileSaveDialog *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)This;return proxy_get_folder(p,ppsi);
}

static HRESULT STDMETHODCALLTYPE save_GetCurrentSelection(IFileSaveDialog *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)This;return IFileSaveDialog_GetCurrentSelection((IFileSaveDialog *)p->variant ,ppsi);
}

static HRESULT STDMETHODCALLTYPE save_SetFileName(IFileSaveDialog *This, LPCWSTR pszName)
{
    file_proxy *p=(file_proxy *)This;return proxy_set_filename(p,pszName);
}

static HRESULT STDMETHODCALLTYPE save_GetFileName(IFileSaveDialog *This, LPWSTR *pszName)
{
    file_proxy *p=(file_proxy *)This;return proxy_get_filename(p,pszName);
}

static HRESULT STDMETHODCALLTYPE save_SetTitle(IFileSaveDialog *This, LPCWSTR pszTitle)
{
    file_proxy *p=(file_proxy *)This;return proxy_title(p,pszTitle);
}

static HRESULT STDMETHODCALLTYPE save_SetOkButtonLabel(IFileSaveDialog *This, LPCWSTR pszText)
{
    file_proxy *p=(file_proxy *)This;return proxy_label(p,pszText,TRUE);
}

static HRESULT STDMETHODCALLTYPE save_SetFileNameLabel(IFileSaveDialog *This, LPCWSTR pszLabel)
{
    file_proxy *p=(file_proxy *)This;return proxy_label(p,pszLabel,FALSE);
}

static HRESULT STDMETHODCALLTYPE save_GetResult(IFileSaveDialog *This, IShellItem **ppsi)
{
    file_proxy *p=(file_proxy *)This;return proxy_result(p,ppsi);
}

static HRESULT STDMETHODCALLTYPE save_AddPlace(IFileSaveDialog *This, IShellItem *psi, FDAP fdap)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_AddPlace((IFileSaveDialog *)p->variant ,psi,fdap);
}

static HRESULT STDMETHODCALLTYPE save_SetDefaultExtension(IFileSaveDialog *This, LPCWSTR pszDefaultExtension)
{
    file_proxy *p=(file_proxy *)This;return proxy_extension(p,pszDefaultExtension);
}

static HRESULT STDMETHODCALLTYPE save_Close(IFileSaveDialog *This, HRESULT hr)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_Close((IFileSaveDialog *)p->variant ,hr);
}

static HRESULT STDMETHODCALLTYPE save_SetClientGuid(IFileSaveDialog *This, REFGUID guid)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_SetClientGuid((IFileSaveDialog *)p->variant ,guid);
}

static HRESULT STDMETHODCALLTYPE save_ClearClientData(IFileSaveDialog *This)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_ClearClientData((IFileSaveDialog *)p->variant);
}

static HRESULT STDMETHODCALLTYPE save_SetFilter(IFileSaveDialog *This, IShellItemFilter *pFilter)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_SetFilter((IFileSaveDialog *)p->variant ,pFilter);
}

static HRESULT STDMETHODCALLTYPE save_SetSaveAsItem(IFileSaveDialog *This, IShellItem *psi)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_SetSaveAsItem((IFileSaveDialog *)p->variant ,psi);
}

static HRESULT STDMETHODCALLTYPE save_SetProperties(IFileSaveDialog *This, IPropertyStore *pStore)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_SetProperties((IFileSaveDialog *)p->variant ,pStore);
}

static HRESULT STDMETHODCALLTYPE save_SetCollectedProperties(IFileSaveDialog *This, IPropertyDescriptionList *pList, WINBOOL fAppendDefault)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_SetCollectedProperties((IFileSaveDialog *)p->variant ,pList,fAppendDefault);
}

static HRESULT STDMETHODCALLTYPE save_GetProperties(IFileSaveDialog *This, IPropertyStore **ppStore)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_GetProperties((IFileSaveDialog *)p->variant ,ppStore);
}

static HRESULT STDMETHODCALLTYPE save_ApplyProperties(IFileSaveDialog *This, IShellItem *psi, IPropertyStore *pStore, HWND hwnd, IFileOperationProgressSink *pSink)
{
    file_proxy *p=(file_proxy *)This;p->unsupported=TRUE;return IFileSaveDialog_ApplyProperties((IFileSaveDialog *)p->variant ,psi,pStore,hwnd,pSink);
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

/* Wrap only interfaces whose complete COM identity we control. */
static BOOL proxy_iid(REFIID iid,BOOL save)
{
    return iid && (IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_IModalWindow)||
        IsEqualIID(iid,&IID_IFileDialog)||IsEqualIID(iid,&IID_IOleWindow)||
        IsEqualIID(iid,&IID_IFileDialog2)||IsEqualIID(iid,&IID_IFileDialogCustomize)||
        (!save && IsEqualIID(iid,&IID_IFileOpenDialog))||(save && IsEqualIID(iid,&IID_IFileSaveDialog)));
}
static file_proxy *proxy_new(IUnknown *original,BOOL save)
{
    IFileDialog *native=NULL;IUnknown *variant=NULL;
    if(FAILED(IUnknown_QueryInterface(original,&IID_IFileDialog,(void **)&native)))return NULL;
    if(FAILED(IUnknown_QueryInterface(original,save?&IID_IFileSaveDialog:&IID_IFileOpenDialog,(void **)&variant))){IFileDialog_Release(native);return NULL;}
    file_proxy *p=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*p));
    if(!p){IUnknown_Release(variant);IFileDialog_Release(native);return NULL;}
    p->vtable=save?(const void *)&save_vtable:(const void *)&open_vtable;
    p->save=save;p->refs=1;p->native=native;p->variant=variant;return p;
}
static void creation_end(void *unused){(void)unused;--native_creation_depth;}
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI bridge_create(REFCLSID clsid,LPUNKNOWN outer,DWORD context,REFIID iid,LPVOID *out)
{
    BOOL nested=native_creation_depth!=0;ntw16s_unwind_scope unwind;HRESULT original_hr;
    ++native_creation_depth;
    NTW16S_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=native_create(clsid,outer,context,iid,out);}
    NTW16S_UNWIND_END(unwind);
    if(FAILED(original_hr)||!out||!*out||outer||nested||ntw16s_modern_dialog_active())return original_hr;
    BOOL save=IsEqualCLSID(clsid,&CLSID_FileSaveDialog),open=IsEqualCLSID(clsid,&CLSID_FileOpenDialog);
    if(!(save||open)||!proxy_iid(iid,save))return original_hr;
    IUnknown *original=(IUnknown *)*out;file_proxy *p=proxy_new(original,save);if(!p)return original_hr;
    void *replacement=NULL;HRESULT hr=proxy_query(p,iid,&replacement);proxy_release(p);
    if(FAILED(hr))return original_hr;
    IUnknown_Release(original);*out=replacement;return original_hr;
}
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI bridge_create_ex(REFCLSID clsid,IUnknown *outer,DWORD context,
    COSERVERINFO *server,DWORD count,MULTI_QI *results)
{
    BOOL nested=native_creation_depth!=0;ntw16s_unwind_scope unwind;HRESULT original_hr;
    ++native_creation_depth;
    NTW16S_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=native_create_ex(clsid,outer,context,server,count,results);}
    NTW16S_UNWIND_END(unwind);
    if(FAILED(original_hr)||outer||server||nested||ntw16s_modern_dialog_active()||!results||!count||count>64)return original_hr;
    BOOL save=IsEqualCLSID(clsid,&CLSID_FileSaveDialog),open=IsEqualCLSID(clsid,&CLSID_FileOpenDialog);
    if(!(save||open))return original_hr;
    IUnknown *original=NULL;
    for(DWORD i=0;i<count;++i){if(SUCCEEDED(results[i].hr) && results[i].pItf){
        if(!proxy_iid(results[i].pIID,save))return original_hr;
        if(!original)original=results[i].pItf;
    }}
    if(!original)return original_hr;
    file_proxy *p=proxy_new(original,save);if(!p)return original_hr;
    void *replacement[64]={0};BOOL complete=TRUE;
    for(DWORD i=0;i<count;++i)if(SUCCEEDED(results[i].hr) && results[i].pItf){
        if(FAILED(proxy_query(p,results[i].pIID,&replacement[i]))){complete=FALSE;break;}
    }
    if(complete){for(DWORD i=0;i<count;++i)if(replacement[i]){
        IUnknown_Release(results[i].pItf);results[i].pItf=(IUnknown *)replacement[i];
    }}else for(DWORD i=0;i<count;++i)if(replacement[i])IUnknown_Release((IUnknown *)replacement[i]);
    proxy_release(p);return original_hr;
}
/* A caller can activate a dialog through the class factory directly. */
typedef struct factory_proxy {IClassFactory iface;LONG refs;IClassFactory *native;BOOL save;} factory_proxy;
static HRESULT STDMETHODCALLTYPE factory_query(IClassFactory *self,REFIID iid,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;if(!IsEqualIID(iid,&IID_IUnknown)&&!IsEqualIID(iid,&IID_IClassFactory))
        return IClassFactory_QueryInterface(((factory_proxy *)self)->native,iid,out);
    *out=self;InterlockedIncrement(&((factory_proxy *)self)->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE factory_add(IClassFactory *self)
{return (ULONG)InterlockedIncrement(&((factory_proxy *)self)->refs);}
static ULONG STDMETHODCALLTYPE factory_release(IClassFactory *self)
{
    factory_proxy *factory=(factory_proxy *)self;LONG count=InterlockedDecrement(&factory->refs);
    if(!count){IClassFactory_Release(factory->native);HeapFree(GetProcessHeap(),0,factory);}
    return (ULONG)count;
}
static NTW16S_UNWIND_FUNCTION HRESULT STDMETHODCALLTYPE factory_create(IClassFactory *self,IUnknown *outer,REFIID iid,void **out)
{
    factory_proxy *factory=(factory_proxy *)self;ntw16s_unwind_scope unwind;HRESULT original_hr;
    ++native_creation_depth;
    NTW16S_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=IClassFactory_CreateInstance(factory->native,outer,iid,out);}
    NTW16S_UNWIND_END(unwind);
    if(FAILED(original_hr)||outer||!out||!*out||ntw16s_modern_dialog_active()||!proxy_iid(iid,factory->save))return original_hr;
    IUnknown *original=(IUnknown *)*out;file_proxy *p=proxy_new(original,factory->save);if(!p)return original_hr;
    void *replacement=NULL;HRESULT hr=proxy_query(p,iid,&replacement);proxy_release(p);
    if(FAILED(hr))return original_hr;
    IUnknown_Release(original);*out=replacement;return original_hr;
}
static HRESULT STDMETHODCALLTYPE factory_lock(IClassFactory *self,BOOL lock)
{return IClassFactory_LockServer(((factory_proxy *)self)->native,lock);}
static IClassFactoryVtbl factory_vtable={factory_query,factory_add,factory_release,factory_create,factory_lock};
static HRESULT WINAPI bridge_class_object(REFCLSID clsid,DWORD context,LPVOID server,REFIID iid,LPVOID *out)
{
    HRESULT original_hr=native_class_object(clsid,context,server,iid,out);
    if(FAILED(original_hr)||!out||!*out||server||native_creation_depth||ntw16s_modern_dialog_active()||
       !(context&CLSCTX_INPROC_SERVER)||!(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_IClassFactory)))return original_hr;
    BOOL save=IsEqualCLSID(clsid,&CLSID_FileSaveDialog),open=IsEqualCLSID(clsid,&CLSID_FileOpenDialog);
    if(!(save||open))return original_hr;
    IUnknown *original=(IUnknown *)*out;IClassFactory *native=NULL;
    if(FAILED(IUnknown_QueryInterface(original,&IID_IClassFactory,(void **)&native)))return original_hr;
    factory_proxy *factory=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*factory));
    if(!factory){IClassFactory_Release(native);return original_hr;}
    factory->iface.lpVtbl=&factory_vtable;factory->refs=1;factory->native=native;factory->save=save;
    IUnknown_Release(original);*out=&factory->iface;return original_hr;
}
LONG ntw16s_attach_com_dialogs(void)
{
    if(!(ntw16s_process_context.flags & NTW16S_POLICY_STYLE))return NO_ERROR;
    native_create=CoCreateInstance;native_create_ex=CoCreateInstanceEx;native_class_object=CoGetClassObject;
    LONG error=(LONG)ntw16s_hooks_attach(&native_create,(void *)bridge_create);
    if(!error)error=(LONG)ntw16s_hooks_attach(&native_create_ex,(void *)bridge_create_ex);
    if(!error)error=(LONG)ntw16s_hooks_attach(&native_class_object,(void *)bridge_class_object);
    return error;
}
