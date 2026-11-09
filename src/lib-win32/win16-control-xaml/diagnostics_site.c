#define COBJMACROS
#include "diagnostics_site.h"
#include "diagnostics_subscribe.h"
#include <objbase.h>
#include <stddef.h>

typedef struct site_object {
    IObjectWithSite site;
    IVisualTreeServiceCallback callback;
    LONG references;
    LONG transitioning;
    IUnknown *owner;
    IUnknown *marshaler;
    IVisualTreeService *service;
    IXamlDiagnostics *diagnostics;
    BOOL connected,advised,asynchronous;
    win16_xaml_site_handlers handlers;
    void *context;
} site_object;

static site_object *from_site(IObjectWithSite *site) { return CONTAINING_RECORD(site,site_object,site); }
static site_object *from_callback(IVisualTreeServiceCallback *callback) { return CONTAINING_RECORD(callback,site_object,callback); }
static ULONG STDMETHODCALLTYPE site_addref(IObjectWithSite *site)
{ return (ULONG)InterlockedIncrement(&from_site(site)->references); }
static HRESULT STDMETHODCALLTYPE site_query(IObjectWithSite *site,REFIID iid,void **out)
{
    if (!out) return E_POINTER;
    *out=NULL;
    if (IsEqualIID(iid,&IID_IMarshal) && from_site(site)->marshaler)
        return IUnknown_QueryInterface(from_site(site)->marshaler,iid,out);
    if (IsEqualIID(iid,&IID_IUnknown) || IsEqualIID(iid,&IID_IObjectWithSite)) *out=site;
    else if (IsEqualIID(iid,&IID_IVisualTreeServiceCallback)) *out=&from_site(site)->callback;
    else return E_NOINTERFACE;
    site_addref(site); return S_OK;
}
static HRESULT site_disconnect(site_object *object)
{
    BOOL had_pin=object->advised;
    if (object->advised) {
        HRESULT hr=IVisualTreeService_UnadviseVisualTreeChange(object->service,&object->callback);
        if (FAILED(hr)) return hr;
        object->advised=FALSE;
    }
    if (object->connected && object->handlers.disconnect) object->handlers.disconnect(object->context);
    if (object->diagnostics) IXamlDiagnostics_Release(object->diagnostics);
    if (object->service) IVisualTreeService_Release(object->service);
    if (object->owner) IUnknown_Release(object->owner);
    object->diagnostics=NULL; object->service=NULL; object->owner=NULL;
    object->connected=FALSE;
    if (had_pin) IObjectWithSite_Release(&object->site);
    return S_OK;
}
static ULONG STDMETHODCALLTYPE site_release(IObjectWithSite *site)
{
    site_object *object=from_site(site);
    LONG references=InterlockedDecrement(&object->references);
    if (!references) {
        (void)site_disconnect(object);
        if (object->marshaler) IUnknown_Release(object->marshaler);
        HeapFree(GetProcessHeap(),0,object);
    }
    return (ULONG)references;
}
static void subscription_complete(void *context,HRESULT hr)
{
    site_object *object=context;
    if (SUCCEEDED(hr)) {
        object->advised=TRUE;
        InterlockedExchange(&object->transitioning,0);
        if (object->handlers.ready) object->handlers.ready(object->context);
    } else {
        if (object->handlers.failed) object->handlers.failed(object->context,hr);
        (void)site_disconnect(object);
        InterlockedExchange(&object->transitioning,0);
        IObjectWithSite_Release(&object->site);
    }
}
static HRESULT STDMETHODCALLTYPE site_set(IObjectWithSite *site,IUnknown *owner)
{
    site_object *object=from_site(site);
    if (InterlockedCompareExchange(&object->transitioning,1,0)!=0) return RPC_E_CALL_REJECTED;
    HRESULT hr=site_disconnect(object);
    if (FAILED(hr) || !owner) goto done;
    hr=IUnknown_QueryInterface(owner,&IID_IVisualTreeService,(void **)&object->service);
    if (SUCCEEDED(hr)) hr=IUnknown_QueryInterface(owner,&IID_IXamlDiagnostics,(void **)&object->diagnostics);
    if (SUCCEEDED(hr) && object->handlers.connect)
        hr=object->handlers.connect(object->context,object->service,object->diagnostics,site);
    if (SUCCEEDED(hr)) {
        object->connected=TRUE;
        IUnknown_AddRef(owner); object->owner=owner;
        /* Keep the callback alive until a successful Unadvise, even if the
         * service does not retain the callback itself. */
        IObjectWithSite_AddRef(&object->site);
        if (object->asynchronous) {
            HANDLE worker=NULL;
            hr=win16_xaml_subscribe_async(object->service,&object->callback,subscription_complete,object,&worker);
            if (SUCCEEDED(hr)) { CloseHandle(worker); return S_OK; }
            IObjectWithSite_Release(&object->site);
            goto failed;
        }
        hr=IVisualTreeService_AdviseVisualTreeChange(object->service,&object->callback);
        if (SUCCEEDED(hr)) {
            object->advised=TRUE;
            if (object->handlers.ready) object->handlers.ready(object->context);
        }
        else IObjectWithSite_Release(&object->site);
    }
failed:
    if (FAILED(hr)) {
        if (object->handlers.failed) object->handlers.failed(object->context,hr);
        site_disconnect(object);
    }
done:
    InterlockedExchange(&object->transitioning,0);
    return hr;
}
static HRESULT STDMETHODCALLTYPE site_get(IObjectWithSite *site,REFIID iid,void **out)
{
    if (!out) return E_POINTER;
    *out=NULL;
    return from_site(site)->owner ? IUnknown_QueryInterface(from_site(site)->owner,iid,out) : E_FAIL;
}
static HRESULT STDMETHODCALLTYPE callback_query(IVisualTreeServiceCallback *callback,REFIID iid,void **out)
{ return site_query(&from_callback(callback)->site,iid,out); }
static ULONG STDMETHODCALLTYPE callback_addref(IVisualTreeServiceCallback *callback)
{ return site_addref(&from_callback(callback)->site); }
static ULONG STDMETHODCALLTYPE callback_release(IVisualTreeServiceCallback *callback)
{ return site_release(&from_callback(callback)->site); }
static HRESULT STDMETHODCALLTYPE callback_change(IVisualTreeServiceCallback *callback,ParentChildRelation relation,
                                                  VisualElement element,VisualMutationType mutation)
{
    site_object *object=from_callback(callback);
    return object->handlers.change ? object->handlers.change(object->context,relation,element,mutation) : S_OK;
}
static IObjectWithSiteVtbl site_vtable={site_query,site_addref,site_release,site_set,site_get};
static IVisualTreeServiceCallbackVtbl callback_vtable={callback_query,callback_addref,callback_release,callback_change};

static HRESULT create_site(const win16_xaml_site_handlers *handlers,void *context,REFIID iid,void **out,BOOL asynchronous)
{
    if (!handlers || !iid || !out) return E_INVALIDARG;
    *out=NULL;
    site_object *object=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*object));
    if (!object) return E_OUTOFMEMORY;
    object->site.lpVtbl=&site_vtable; object->callback.lpVtbl=&callback_vtable;
    object->references=1; object->handlers=*handlers; object->context=context; object->asynchronous=asynchronous;
    HRESULT hr=asynchronous ? CoCreateFreeThreadedMarshaler((IUnknown *)&object->site,&object->marshaler) : S_OK;
    if (SUCCEEDED(hr)) hr=site_query(&object->site,iid,out);
    site_release(&object->site);
    return hr;
}
HRESULT win16_xaml_site_create(const win16_xaml_site_handlers *handlers,void *context,REFIID iid,void **out)
{ return create_site(handlers,context,iid,out,FALSE); }
HRESULT win16_xaml_site_create_async(const win16_xaml_site_handlers *handlers,void *context,REFIID iid,void **out)
{ return create_site(handlers,context,iid,out,TRUE); }
