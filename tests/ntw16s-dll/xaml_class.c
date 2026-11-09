#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <ocidl.h>
#include <xamlom.h>
#include <stdio.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
typedef HRESULT (STDAPICALLTYPE *class_object_fn)(REFCLSID,REFIID,void **);
#ifdef XAML_DIRECT_STOP
HRESULT STDAPICALLTYPE DllGetClassObject(REFCLSID,REFIID,void **);
HRESULT ntw16s_xaml_stop(void);
#endif
typedef struct fake_owner {
    IUnknown unknown;
    IVisualTreeService service;
    IXamlDiagnostics diagnostics;
    LONG refs;
    IUnknown *marshaler;
    IVisualTreeServiceCallback *callback;
    HANDLE ready;
} fake_owner;
static ULONG STDMETHODCALLTYPE add(IUnknown *p) { return (ULONG)InterlockedIncrement(&((fake_owner *)p)->refs); }
static ULONG STDMETHODCALLTYPE release(IUnknown *p) { return (ULONG)InterlockedDecrement(&((fake_owner *)p)->refs); }
static HRESULT STDMETHODCALLTYPE query(IUnknown *p,REFIID iid,void **out)
{
    fake_owner *o=(fake_owner *)p;
    if (!out) return E_POINTER;
    *out=NULL;
    if (IsEqualIID(iid,&IID_IMarshal) && o->marshaler) return IUnknown_QueryInterface(o->marshaler,iid,out);
    if (IsEqualIID(iid,&IID_IUnknown)) *out=&o->unknown;
    else if (IsEqualIID(iid,&IID_IVisualTreeService)) *out=&o->service;
    else if (IsEqualIID(iid,&IID_IXamlDiagnostics)) *out=&o->diagnostics;
    else return E_NOINTERFACE;
    add(p); return S_OK;
}
static fake_owner *service_owner(IVisualTreeService *p) { return CONTAINING_RECORD(p,fake_owner,service); }
static fake_owner *diag_owner(IXamlDiagnostics *p) { return CONTAINING_RECORD(p,fake_owner,diagnostics); }
static HRESULT STDMETHODCALLTYPE service_query(IVisualTreeService *p,REFIID i,void **o) { return query(&service_owner(p)->unknown,i,o); }
static ULONG STDMETHODCALLTYPE service_add(IVisualTreeService *p) { return add(&service_owner(p)->unknown); }
static ULONG STDMETHODCALLTYPE service_release(IVisualTreeService *p) { return release(&service_owner(p)->unknown); }
static HRESULT STDMETHODCALLTYPE advise(IVisualTreeService *p,IVisualTreeServiceCallback *callback)
{
    fake_owner *o=service_owner(p);
    IVisualTreeServiceCallback_AddRef(callback); o->callback=callback;
    SetEvent(o->ready); return S_OK;
}
static HRESULT STDMETHODCALLTYPE unadvise(IVisualTreeService *p,IVisualTreeServiceCallback *callback)
{
    fake_owner *o=service_owner(p);
    if (o->callback!=callback) return E_INVALIDARG;
    o->callback=NULL; IVisualTreeServiceCallback_Release(callback); return S_OK;
}
static HRESULT STDMETHODCALLTYPE diag_query(IXamlDiagnostics *p,REFIID i,void **o) { return query(&diag_owner(p)->unknown,i,o); }
static ULONG STDMETHODCALLTYPE diag_add(IXamlDiagnostics *p) { return add(&diag_owner(p)->unknown); }
static ULONG STDMETHODCALLTYPE diag_release(IXamlDiagnostics *p) { return release(&diag_owner(p)->unknown); }
static IUnknownVtbl owner_vtable={query,add,release};
static IVisualTreeServiceVtbl service_vtable={.QueryInterface=service_query,.AddRef=service_add,.Release=service_release,.AdviseVisualTreeChange=advise,.UnadviseVisualTreeChange=unadvise};
static IXamlDiagnosticsVtbl diag_vtable={.QueryInterface=diag_query,.AddRef=diag_add,.Release=diag_release};
int wmain(int argc,WCHAR **argv)
{
    static const CLSID supported={0x7132a744,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};
    static const CLSID other={0x7132a745,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};
#ifdef XAML_DIRECT_STOP
    (void)argc; (void)argv;
#else
    CHECK(argc==2);
#endif
    CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_MULTITHREADED)));
#ifdef XAML_DIRECT_STOP
    class_object_fn get=DllGetClassObject;
#else
    HMODULE module=LoadLibraryW(argv[1]); CHECK(module);
    class_object_fn get=(class_object_fn)(void *)GetProcAddress(module,"DllGetClassObject"); CHECK(get);
#endif
    IClassFactory *factory=NULL;
    CHECK(get(&other,&IID_IClassFactory,(void **)&factory)==CLASS_E_CLASSNOTAVAILABLE && !factory);
    CHECK(get(&supported,&IID_IClassFactory,(void **)&factory)==S_OK && factory);
    IObjectWithSite *site=NULL;
    CHECK(IClassFactory_CreateInstance(factory,NULL,&IID_IObjectWithSite,(void **)&site)==S_OK && site);
    fake_owner owner={{&owner_vtable},{&service_vtable},{&diag_vtable},1,NULL,NULL,NULL};
    owner.ready=CreateEventW(NULL,TRUE,FALSE,NULL); CHECK(owner.ready);
    CHECK(CoCreateFreeThreadedMarshaler(&owner.unknown,&owner.marshaler)==S_OK);
    CHECK(IObjectWithSite_SetSite(site,&owner.unknown)==S_OK);
    CHECK(WaitForSingleObject(owner.ready,2000)==WAIT_OBJECT_0);
    /* The notification has no resolvable object: no XAML state is changed. */
    ParentChildRelation relation={0}; VisualElement element={0};
    CHECK(IVisualTreeServiceCallback_OnVisualTreeChange(owner.callback,relation,element,Add)==S_OK);
#ifdef XAML_DIRECT_STOP
    CHECK(ntw16s_xaml_stop()==S_OK);
    for (int i=0;i<200 && owner.callback;i++) Sleep(10);
    CHECK(!owner.callback);
#else
    HRESULT stop=RPC_E_CALL_REJECTED;
    for (int i=0;i<200 && stop==RPC_E_CALL_REJECTED;i++) {
        stop=IObjectWithSite_SetSite(site,NULL);
        if (stop==RPC_E_CALL_REJECTED) Sleep(10);
    }
    CHECK(stop==S_OK && !owner.callback);
#endif
    CHECK(IObjectWithSite_Release(site)==0);
    for (int i=0;i<200 && InterlockedCompareExchange(&owner.refs,0,0)!=1;i++) Sleep(10);
    CHECK(owner.refs==1);
    IUnknown_Release(owner.marshaler); CloseHandle(owner.ready);
    CHECK(IClassFactory_Release(factory)==0);
#ifndef XAML_DIRECT_STOP
    CHECK(FreeLibrary(module));
#endif
    CoUninitialize();
    return 0;
}
