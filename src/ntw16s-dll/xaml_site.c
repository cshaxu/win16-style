#define COBJMACROS
#define INITGUID
#include "lib-win32/win16-control-xaml/adapter.h"
#include "lib-win32/win16-control-xaml/diagnostics_factory.h"
#include "lib-win32/win16-control-xaml/diagnostics_start.h"
#include "xaml_site.h"
#include <stdint.h>
#include <roapi.h>

/* This class is only instantiated by a caller of XAML Diagnostics.  The
 * current adapter supports one UI thread and must outlive its callbacks. */
static const CLSID xaml_class={0x7132a744,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};
static win16_xaml_adapter adapter;
static LONG last_xaml_error;
static IObjectWithSite *active_site;
static SRWLOCK site_lock=SRWLOCK_INIT;
static HANDLE session_stop;

typedef struct xaml_worker {
    HMODULE self;
    HANDLE stop;
    WCHAR loader[32768],xaml[32768],tap[32768];
} xaml_worker;

static DWORD WINAPI start_worker(void *context)
{
    xaml_worker *worker=context;
    HRESULT apartment=RoInitialize(RO_INIT_MULTITHREADED);
    HMODULE loader=SUCCEEDED(apartment) ? LoadLibraryExW(worker->loader,NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS) : NULL;
    HRESULT hr=loader ? win16_xaml_diagnostics_start(GetCurrentProcessId(),worker->loader,
        worker->xaml,worker->tap,L"WinUIVisualDiagConnection1",&xaml_class) : E_FAIL;
    if (SUCCEEDED(hr)) (void)WaitForSingleObject(worker->stop,INFINITE);
    AcquireSRWLockExclusive(&site_lock);
    if (session_stop==worker->stop) session_stop=NULL;
    ReleaseSRWLockExclusive(&site_lock);
    CloseHandle(worker->stop);
    if (loader) FreeLibrary(loader);
    if (SUCCEEDED(apartment)) RoUninitialize();
    HMODULE self=worker->self;
    HeapFree(GetProcessHeap(),0,worker);
    FreeLibraryAndExitThread(self,0);
}

void ntw16s_xaml_start(void)
{
    AcquireSRWLockShared(&site_lock);
    BOOL active=session_stop || active_site;
    ReleaseSRWLockShared(&site_lock);
    if (active) return;
    HMODULE loader=GetModuleHandleW(L"Microsoft.Internal.FrameworkUdk.dll");
    HMODULE xaml=GetModuleHandleW(L"Microsoft.UI.Xaml.dll"),self=NULL;
    if (!loader || !xaml || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (PCWSTR)(uintptr_t)ntw16s_xaml_start,&self)) return;
    xaml_worker *worker=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*worker));
    if (!worker) { FreeLibrary(self); return; }
    worker->self=self;
    DWORD a=GetModuleFileNameW(loader,worker->loader,ARRAYSIZE(worker->loader));
    DWORD b=GetModuleFileNameW(xaml,worker->xaml,ARRAYSIZE(worker->xaml));
    DWORD c=GetModuleFileNameW(self,worker->tap,ARRAYSIZE(worker->tap));
    if (!a || a>=ARRAYSIZE(worker->loader) || !b || b>=ARRAYSIZE(worker->xaml) ||
        !c || c>=ARRAYSIZE(worker->tap)) { HeapFree(GetProcessHeap(),0,worker); FreeLibrary(self); return; }
    worker->stop=CreateEventW(NULL,TRUE,FALSE,NULL);
    if (!worker->stop) { HeapFree(GetProcessHeap(),0,worker); FreeLibrary(self); return; }
    AcquireSRWLockExclusive(&site_lock);
    HANDLE thread=NULL;
    if (!session_stop && !active_site) {
        session_stop=worker->stop;
        thread=CreateThread(NULL,0,start_worker,worker,0,NULL);
        if (!thread) session_stop=NULL;
    }
    ReleaseSRWLockExclusive(&site_lock);
    if (thread) CloseHandle(thread);
    else { CloseHandle(worker->stop); HeapFree(GetProcessHeap(),0,worker); FreeLibrary(self); }
}

static HRESULT connect_site(void *context,IVisualTreeService *service,IXamlDiagnostics *diagnostics,IObjectWithSite *site)
{
    AcquireSRWLockExclusive(&site_lock);
    HRESULT hr=session_stop && WaitForSingleObject(session_stop,0)==WAIT_OBJECT_0
        ? E_ABORT : win16_xaml_adapter_connect(context,service,diagnostics);
    if (SUCCEEDED(hr)) {
        IObjectWithSite_AddRef(site);
        active_site=site;
    }
    ReleaseSRWLockExclusive(&site_lock);
    return hr;
}
static void disconnect_site(void *context)
{
    win16_xaml_adapter_disconnect(context);
    AcquireSRWLockExclusive(&site_lock);
    IObjectWithSite *site=active_site;
    active_site=NULL;
    if (session_stop) SetEvent(session_stop);
    ReleaseSRWLockExclusive(&site_lock);
    if (site) IObjectWithSite_Release(site);
}
static DWORD WINAPI disconnect_worker(void *context)
{
    IObjectWithSite *site=context;
    HRESULT initialized=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if (SUCCEEDED(initialized)) {
        HRESULT hr=RPC_E_CALL_REJECTED;
        for (unsigned int i=0;i<200 && hr==RPC_E_CALL_REJECTED;i++) {
            hr=IObjectWithSite_SetSite(site,NULL);
            if (hr==RPC_E_CALL_REJECTED) Sleep(10);
        }
        CoUninitialize();
    }
    IObjectWithSite_Release(site);
    return 0;
}
HRESULT ntw16s_xaml_remove_window(HWND window)
{ return win16_xaml_adapter_remove_window(&adapter,window); }
HRESULT ntw16s_xaml_stop(void)
{
    AcquireSRWLockShared(&site_lock);
    IObjectWithSite *site=active_site;
    if (site) IObjectWithSite_AddRef(site);
    ReleaseSRWLockShared(&site_lock);
    if (!site) {
        AcquireSRWLockShared(&site_lock);
        if (session_stop) SetEvent(session_stop);
        ReleaseSRWLockShared(&site_lock);
        return S_FALSE;
    }
    HRESULT hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) {
        HANDLE worker=CreateThread(NULL,0,disconnect_worker,site,0,NULL);
        if (worker) { CloseHandle(worker); return S_OK; }
        hr=HRESULT_FROM_WIN32(GetLastError());
    }
    IObjectWithSite_Release(site);
    return hr;
}
static HRESULT change_site(void *context,ParentChildRelation relation,VisualElement element,VisualMutationType mutation)
{
    /* A failed observation must not abort XAML's visual tree notification. */
    HRESULT hr=win16_xaml_adapter_change(context,relation,element,mutation,NULL);
    if (FAILED(hr)) InterlockedExchange(&last_xaml_error,(LONG)hr);
    return S_OK;
}
HRESULT STDAPICALLTYPE DllGetClassObject(REFCLSID clsid,REFIID iid,void **out)
{
    if (!out) return E_POINTER;
    *out=NULL;
    if (!clsid || !iid) return E_INVALIDARG;
    if (!IsEqualCLSID(clsid,&xaml_class)) return CLASS_E_CLASSNOTAVAILABLE;
    HMODULE module=NULL;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                            (PCWSTR)(uintptr_t)DllGetClassObject,&module))
        return HRESULT_FROM_WIN32(GetLastError());
    const win16_xaml_site_handlers handlers={connect_site,disconnect_site,change_site,NULL,NULL};
    return win16_xaml_get_class_object(clsid,&xaml_class,&handlers,&adapter,iid,out);
}
