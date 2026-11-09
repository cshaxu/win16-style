#define COBJMACROS
#include "diagnostics_factory.h"

typedef struct factory {
    IClassFactory iface;
    LONG references;
    win16_xaml_site_handlers handlers;
    void *context;
} factory;
static ULONG STDMETHODCALLTYPE add_ref(IClassFactory *iface)
{ return (ULONG)InterlockedIncrement(&((factory *)iface)->references); }
static HRESULT STDMETHODCALLTYPE query(IClassFactory *iface,REFIID iid,void **out)
{
    if (!out) return E_POINTER;
    *out=NULL;
    if (!IsEqualIID(iid,&IID_IUnknown) && !IsEqualIID(iid,&IID_IClassFactory)) return E_NOINTERFACE;
    *out=iface; add_ref(iface); return S_OK;
}
static ULONG STDMETHODCALLTYPE release(IClassFactory *iface)
{
    factory *f=(factory *)iface; LONG count=InterlockedDecrement(&f->references);
    if (!count) HeapFree(GetProcessHeap(),0,f);
    return (ULONG)count;
}
static HRESULT STDMETHODCALLTYPE create(IClassFactory *iface,IUnknown *outer,REFIID iid,void **out)
{
    if (outer) { if (out) *out=NULL; return CLASS_E_NOAGGREGATION; }
    factory *f=(factory *)iface;
    return win16_xaml_site_create_async(&f->handlers,f->context,iid,out);
}
static HRESULT STDMETHODCALLTYPE lock(IClassFactory *iface,BOOL locked)
{ (void)iface; (void)locked; return E_NOTIMPL; }
static IClassFactoryVtbl vtable={query,add_ref,release,create,lock};

HRESULT win16_xaml_get_class_object(REFCLSID requested,REFCLSID supported,
                                    const win16_xaml_site_handlers *handlers,void *context,
                                    REFIID iid,void **out)
{
    if (!out) return E_POINTER;
    *out=NULL;
    if (!requested || !supported || !handlers || !iid) return E_INVALIDARG;
    if (!IsEqualCLSID(requested,supported)) return CLASS_E_CLASSNOTAVAILABLE;
    factory *f=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*f));
    if (!f) return E_OUTOFMEMORY;
    f->iface.lpVtbl=&vtable; f->references=1; f->handlers=*handlers; f->context=context;
    HRESULT hr=query(&f->iface,iid,out);
    release(&f->iface);
    return hr;
}
