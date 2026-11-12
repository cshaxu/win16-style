/* Forward native file-dialog events while preserving the owning COM identity. */
#define COBJMACROS
#include "lib-style/common/file_events.h"
#include <stdint.h>

static HRESULT STDMETHODCALLTYPE event_query(IFileDialogEvents *self,REFIID iid,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;if(!IsEqualIID(iid,&IID_IUnknown) && !IsEqualIID(iid,&IID_IFileDialogEvents))return E_NOINTERFACE;
    *out=self;InterlockedIncrement(&((style_dialog_event_adapter *)self)->refs);return S_OK;
}
ULONG STDMETHODCALLTYPE style_dialog_event_add(IFileDialogEvents *self)
{return (ULONG)InterlockedIncrement(&((style_dialog_event_adapter *)self)->refs);}
ULONG STDMETHODCALLTYPE style_dialog_event_release(IFileDialogEvents *self)
{
    style_dialog_event_adapter *event=(style_dialog_event_adapter *)self;LONG refs=InterlockedDecrement(&event->refs);
    if(!refs){IFileDialogEvents_Release(event->target);HeapFree(GetProcessHeap(),0,event);}
    return (ULONG)refs;
}
static IFileDialog *event_dialog(style_dialog_event_adapter *event,IFileDialog *native,void **held)
{
    *held=event->owner;if(*held){event->services->add(*held);return (IFileDialog *)*held;}return native;
}
#define EVENT_SIMPLE(name,method) \
static HRESULT STDMETHODCALLTYPE name(IFileDialogEvents *self,IFileDialog *native) \
{style_dialog_event_adapter *e=(style_dialog_event_adapter *)self;style_dialog_event_add(self);void *held;IFileDialog *d=event_dialog(e,native,&held); \
 HRESULT hr=IFileDialogEvents_##method(e->target,d);if(held)e->services->release(held);style_dialog_event_release(self);return hr;}
EVENT_SIMPLE(event_ok,OnFileOk)
EVENT_SIMPLE(event_folder,OnFolderChange)
EVENT_SIMPLE(event_selection,OnSelectionChange)
EVENT_SIMPLE(event_type,OnTypeChange)
#undef EVENT_SIMPLE
static HRESULT STDMETHODCALLTYPE event_changing(IFileDialogEvents *self,IFileDialog *native,IShellItem *item)
{
    style_dialog_event_adapter *e=(style_dialog_event_adapter *)self;style_dialog_event_add(self);void *held;IFileDialog *d=event_dialog(e,native,&held);
    HRESULT hr=IFileDialogEvents_OnFolderChanging(e->target,d,item);if(held)e->services->release(held);style_dialog_event_release(self);return hr;
}
static HRESULT STDMETHODCALLTYPE event_share(IFileDialogEvents *self,IFileDialog *native,IShellItem *item,FDE_SHAREVIOLATION_RESPONSE *response)
{
    style_dialog_event_adapter *e=(style_dialog_event_adapter *)self;style_dialog_event_add(self);void *held;IFileDialog *d=event_dialog(e,native,&held);
    HRESULT hr=IFileDialogEvents_OnShareViolation(e->target,d,item,response);if(held)e->services->release(held);style_dialog_event_release(self);return hr;
}
static HRESULT STDMETHODCALLTYPE event_overwrite(IFileDialogEvents *self,IFileDialog *native,IShellItem *item,FDE_OVERWRITE_RESPONSE *response)
{
    style_dialog_event_adapter *e=(style_dialog_event_adapter *)self;style_dialog_event_add(self);void *held;IFileDialog *d=event_dialog(e,native,&held);
    HRESULT hr=IFileDialogEvents_OnOverwrite(e->target,d,item,response);if(held)e->services->release(held);style_dialog_event_release(self);return hr;
}
static IFileDialogEventsVtbl event_vtable={event_query,style_dialog_event_add,style_dialog_event_release,event_ok,event_changing,event_folder,event_selection,event_share,event_type,event_overwrite};

style_dialog_event_adapter *style_dialog_event_new(void *owner,IFileDialogEvents *target,const style_dialog_event_services *services)
{
    style_dialog_event_adapter *event=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*event));
    if(!event)return NULL;
    event->iface.lpVtbl=&event_vtable;event->refs=1;event->owner=owner;event->target=target;event->services=services;
    IFileDialogEvents_AddRef(target);return event;
}

HRESULT style_dialog_events_advise(IFileDialog *native,style_dialog_event_adapter **events,void *owner,
    IFileDialogEvents *target,DWORD *cookie,const style_dialog_event_services *services)
{
    if(!target||!cookie)return E_INVALIDARG;
    style_dialog_event_adapter *e=style_dialog_event_new(owner,target,services);if(!e)return E_OUTOFMEMORY;
    HRESULT hr=IFileDialog_Advise(native,&e->iface,cookie);
    if(FAILED(hr)){e->owner=NULL;style_dialog_event_release(&e->iface);return hr;}
    e->cookie=*cookie;e->next=*events;*events=e;return hr;
}
HRESULT style_dialog_events_unadvise(IFileDialog *native,style_dialog_event_adapter **events,DWORD cookie)
{
    HRESULT hr=IFileDialog_Unadvise(native,cookie);if(FAILED(hr))return hr;
    style_dialog_event_adapter **link=events;
    while(*link && (*link)->cookie!=cookie)link=&(*link)->next;
    if(*link){style_dialog_event_adapter *e=*link;*link=e->next;e->owner=NULL;style_dialog_event_release(&e->iface);}
    return hr;
}
void style_dialog_events_release(IFileDialog *native,style_dialog_event_adapter **events)
{
    while(*events){style_dialog_event_adapter *e=*events;*events=e->next;e->owner=NULL;
        (void)IFileDialog_Unadvise(native,e->cookie);style_dialog_event_release(&e->iface);}
}

/* Hold each listener independently while an application callback may mutate the list. */
BOOL style_dialog_event_snapshot_init(style_dialog_event_adapter *events,style_dialog_event_snapshot *snapshot)
{
    size_t count=0;for(style_dialog_event_adapter *e=events;e;e=e->next)++count;
    snapshot->items=NULL;snapshot->count=0;if(!count)return TRUE;
    if(count>SIZE_MAX/sizeof(*snapshot->items))return FALSE;
    snapshot->items=HeapAlloc(GetProcessHeap(),0,count*sizeof(*snapshot->items));
    if(!snapshot->items)return FALSE;
    snapshot->count=count;
    for(style_dialog_event_adapter *e=events;e;e=e->next){style_dialog_event_add(&e->iface);snapshot->items[--count]=e;}
    return TRUE;
}
void style_dialog_event_snapshot_free(style_dialog_event_snapshot *snapshot)
{
    for(size_t i=0;i<snapshot->count;++i)style_dialog_event_release(&snapshot->items[i]->iface);
    HeapFree(GetProcessHeap(),0,snapshot->items);snapshot->items=NULL;snapshot->count=0;
}

HRESULT style_dialog_event_dispatch(const style_dialog_event_snapshot *snapshot,void *owner,enum style_classic_event_kind kind,IShellItem *item)
{
    HRESULT result=S_OK;
    for(size_t i=0;i<snapshot->count;++i){style_dialog_event_adapter *e=snapshot->items[i];if(e->owner!=owner)continue;
        HRESULT hr=kind==STYLE_CLASSIC_OK?IFileDialogEvents_OnFileOk(e->target,(IFileDialog *)owner):
            kind==STYLE_CLASSIC_FOLDER_CHANGING?IFileDialogEvents_OnFolderChanging(e->target,(IFileDialog *)owner,item):
            kind==STYLE_CLASSIC_FOLDER_CHANGED?IFileDialogEvents_OnFolderChange(e->target,(IFileDialog *)owner):
            kind==STYLE_CLASSIC_SELECTION_CHANGED?IFileDialogEvents_OnSelectionChange(e->target,(IFileDialog *)owner):
            IFileDialogEvents_OnTypeChange(e->target,(IFileDialog *)owner);
        if(kind==STYLE_CLASSIC_OK && hr!=S_OK && hr!=E_NOTIMPL){result=FAILED(hr)?hr:S_FALSE;break;}
        if(kind==STYLE_CLASSIC_FOLDER_CHANGING && hr!=S_OK && hr!=E_NOTIMPL){result=hr;break;}
    }
    return result;
}
FDE_OVERWRITE_RESPONSE style_dialog_event_overwrite(const style_dialog_event_snapshot *snapshot,void *owner,IShellItem *item)
{
    for(size_t i=0;i<snapshot->count;++i){style_dialog_event_adapter *e=snapshot->items[i];
        if(e->owner!=owner)continue;
        FDE_OVERWRITE_RESPONSE response=FDEOR_DEFAULT;
        HRESULT hr=IFileDialogEvents_OnOverwrite(e->target,(IFileDialog *)owner,item,&response);
        if(SUCCEEDED(hr))return response;
    }
    return FDEOR_DEFAULT;
}
FDE_SHAREVIOLATION_RESPONSE style_dialog_event_share(const style_dialog_event_snapshot *snapshot,void *owner,IShellItem *item)
{
    for(size_t i=0;i<snapshot->count;++i){style_dialog_event_adapter *e=snapshot->items[i];
        if(e->owner!=owner)continue;
        FDE_SHAREVIOLATION_RESPONSE response=FDESVR_DEFAULT;
        HRESULT hr=IFileDialogEvents_OnShareViolation(e->target,(IFileDialog *)owner,item,&response);
        if(SUCCEEDED(hr))return response;
    }
    return FDESVR_DEFAULT;
}
