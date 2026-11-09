#define COBJMACROS
#define INITGUID
#include "lib-win32/win16-control-xaml/diagnostics_site.h"
#include <stdio.h>

typedef struct fake_owner {
    IUnknown unknown;
    IVisualTreeService service;
    IXamlDiagnostics diagnostics;
    LONG refs;
    IVisualTreeServiceCallback *callback;
    BOOL fail_advise,fail_unadvise;
    HRESULT nested_result;
    IUnknown *marshaler;
    HANDLE entered,proceed;
} fake_owner;
static fake_owner *from_service(IVisualTreeService *p) { return CONTAINING_RECORD(p,fake_owner,service); }
static fake_owner *from_diagnostics(IXamlDiagnostics *p) { return CONTAINING_RECORD(p,fake_owner,diagnostics); }
static ULONG STDMETHODCALLTYPE owner_add(IUnknown *p) { return (ULONG)InterlockedIncrement(&((fake_owner *)p)->refs); }
static ULONG STDMETHODCALLTYPE owner_release(IUnknown *p) { return (ULONG)InterlockedDecrement(&((fake_owner *)p)->refs); }
static HRESULT STDMETHODCALLTYPE owner_query(IUnknown *p,REFIID iid,void **out)
{
    fake_owner *o=(fake_owner *)p; if (!out) return E_POINTER; *out=NULL;
    if (IsEqualIID(iid,&IID_IMarshal) && o->marshaler) return IUnknown_QueryInterface(o->marshaler,iid,out);
    if (IsEqualIID(iid,&IID_IUnknown)) *out=&o->unknown;
    else if (IsEqualIID(iid,&IID_IVisualTreeService)) *out=&o->service;
    else if (IsEqualIID(iid,&IID_IXamlDiagnostics)) *out=&o->diagnostics;
    else return E_NOINTERFACE;
    owner_add(p); return S_OK;
}
static HRESULT STDMETHODCALLTYPE service_query(IVisualTreeService *p,REFIID i,void **o) { return owner_query(&from_service(p)->unknown,i,o); }
static ULONG STDMETHODCALLTYPE service_add(IVisualTreeService *p) { return owner_add(&from_service(p)->unknown); }
static ULONG STDMETHODCALLTYPE service_release(IVisualTreeService *p) { return owner_release(&from_service(p)->unknown); }
static HRESULT STDMETHODCALLTYPE service_advise(IVisualTreeService *p,IVisualTreeServiceCallback *callback)
{
    fake_owner *o=from_service(p); if (o->fail_advise) return E_FAIL;
    if (o->entered && o->proceed) {
        SetEvent(o->entered);
        if (WaitForSingleObject(o->proceed,2000)!=WAIT_OBJECT_0) return E_FAIL;
    }
    IObjectWithSite *site=NULL;
    if (SUCCEEDED(IVisualTreeServiceCallback_QueryInterface(callback,&IID_IObjectWithSite,(void **)&site))) {
        o->nested_result=IObjectWithSite_SetSite(site,NULL);
        IObjectWithSite_Release(site);
    }
    o->callback=callback; IVisualTreeServiceCallback_AddRef(callback); return S_OK;
}
static HRESULT STDMETHODCALLTYPE service_unadvise(IVisualTreeService *p,IVisualTreeServiceCallback *callback)
{
    fake_owner *o=from_service(p); if (o->fail_unadvise) return E_FAIL;
    if (o->callback!=callback) return E_INVALIDARG;
    o->callback=NULL; IVisualTreeServiceCallback_Release(callback); return S_OK;
}
static HRESULT STDMETHODCALLTYPE diag_query(IXamlDiagnostics *p,REFIID i,void **o) { return owner_query(&from_diagnostics(p)->unknown,i,o); }
static ULONG STDMETHODCALLTYPE diag_add(IXamlDiagnostics *p) { return owner_add(&from_diagnostics(p)->unknown); }
static ULONG STDMETHODCALLTYPE diag_release(IXamlDiagnostics *p) { return owner_release(&from_diagnostics(p)->unknown); }
static IUnknownVtbl owner_vtable={owner_query,owner_add,owner_release};
static IVisualTreeServiceVtbl service_vtable={.QueryInterface=service_query,.AddRef=service_add,.Release=service_release,.AdviseVisualTreeChange=service_advise,.UnadviseVisualTreeChange=service_unadvise};
static IXamlDiagnosticsVtbl diag_vtable={.QueryInterface=diag_query,.AddRef=diag_add,.Release=diag_release};
typedef struct record { int connected,disconnected,changes; HANDLE ready; } record;
static HRESULT connected(void *ctx,IVisualTreeService *service,IXamlDiagnostics *diagnostics,IObjectWithSite *site)
{ (void)service; (void)diagnostics; (void)site; ((record *)ctx)->connected++; return S_OK; }
static void disconnected(void *ctx) { ((record *)ctx)->disconnected++; }
static HRESULT changed(void *ctx,ParentChildRelation relation,VisualElement element,VisualMutationType mutation)
{ (void)relation; (void)element; (void)mutation; ((record *)ctx)->changes++; return S_OK; }
static void ready(void *ctx) { if (((record *)ctx)->ready) SetEvent(((record *)ctx)->ready); }
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr,"line %d: %s\n",__LINE__,#expr); return 1; } } while (0)
int main(void)
{
    fake_owner owner={{&owner_vtable},{&service_vtable},{&diag_vtable},1,NULL,FALSE,FALSE,S_OK,NULL,NULL,NULL};
    record rec={0}; const win16_xaml_site_handlers handlers={connected,disconnected,changed,ready,NULL};
    IObjectWithSite *site=NULL;
    CHECK(win16_xaml_site_create(&handlers,&rec,&IID_IObjectWithSite,(void **)&site)==S_OK);
    owner.fail_advise=TRUE;
    CHECK(IObjectWithSite_SetSite(site,&owner.unknown)==E_FAIL);
    CHECK(owner.refs==1 && !owner.callback && rec.connected==1 && rec.disconnected==1);
    owner.fail_advise=FALSE;
    CHECK(IObjectWithSite_SetSite(site,&owner.unknown)==S_OK && owner.callback);
    CHECK(owner.nested_result==RPC_E_CALL_REJECTED);
    ParentChildRelation relation={0}; VisualElement element={0};
    CHECK(IVisualTreeServiceCallback_OnVisualTreeChange(owner.callback,relation,element,Add)==S_OK && rec.changes==1);
    owner.fail_unadvise=TRUE;
    CHECK(IObjectWithSite_SetSite(site,NULL)==E_FAIL && owner.callback && rec.disconnected==1);
    owner.fail_unadvise=FALSE;
    CHECK(IObjectWithSite_SetSite(site,NULL)==S_OK && !owner.callback && owner.refs==1 && rec.disconnected==2);
    CHECK(IObjectWithSite_Release(site)==0);
    CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_MULTITHREADED)));
    site=NULL;
    CHECK(win16_xaml_site_create_async(&handlers,&rec,&IID_IObjectWithSite,(void **)&site)==S_OK);
    IMarshal *marshaler=NULL;
    CHECK(IObjectWithSite_QueryInterface(site,&IID_IMarshal,(void **)&marshaler)==S_OK && marshaler);
    IMarshal_Release(marshaler);
    CHECK(CoCreateFreeThreadedMarshaler(&owner.unknown,&owner.marshaler)==S_OK);
    owner.entered=CreateEventW(NULL,TRUE,FALSE,NULL);owner.proceed=CreateEventW(NULL,TRUE,FALSE,NULL);
    rec.ready=CreateEventW(NULL,TRUE,FALSE,NULL);
    CHECK(owner.entered && owner.proceed && rec.ready);
    CHECK(IObjectWithSite_SetSite(site,&owner.unknown)==S_OK);
    CHECK(WaitForSingleObject(owner.entered,2000)==WAIT_OBJECT_0);
    CHECK(IObjectWithSite_SetSite(site,NULL)==RPC_E_CALL_REJECTED);
    CHECK(SetEvent(owner.proceed));
    CHECK(WaitForSingleObject(rec.ready,2000)==WAIT_OBJECT_0);
    CHECK(IObjectWithSite_SetSite(site,NULL)==S_OK);
    IObjectWithSite_Release(site);
    for (unsigned int i=0;i<200 && InterlockedCompareExchange(&owner.refs,0,0)!=1;i++) Sleep(10);
    CHECK(owner.refs==1);
    IUnknown_Release(owner.marshaler); CloseHandle(owner.entered); CloseHandle(owner.proceed); CloseHandle(rec.ready);
    CoUninitialize();
    return 0;
}
