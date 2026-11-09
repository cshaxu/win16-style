/* Native collection test: mock XAML operations, real collection and COM identity. */
#define COBJMACROS
#include "lib-win32/win16-control-xaml/button_style.h"
#include <stdio.h>

typedef struct fake_button { IInspectable iface; LONG references; } fake_button;
static HRESULT STDMETHODCALLTYPE fake_query(IInspectable *self, REFIID iid, void **out)
{
    (void)iid;
    if (!out) return E_POINTER;
    *out=self; IInspectable_AddRef(self); return S_OK;
}
static ULONG STDMETHODCALLTYPE fake_addref(IInspectable *self)
{ return (ULONG)InterlockedIncrement(&((fake_button *)self)->references); }
static ULONG STDMETHODCALLTYPE fake_release(IInspectable *self)
{ return (ULONG)InterlockedDecrement(&((fake_button *)self)->references); }
static HRESULT STDMETHODCALLTYPE fake_iids(IInspectable *self, ULONG *count, IID **iids)
{ (void)self; (void)count; (void)iids; return E_NOTIMPL; }
static HRESULT STDMETHODCALLTYPE fake_name(IInspectable *self, HSTRING *name)
{ (void)self; (void)name; return E_NOTIMPL; }
static HRESULT STDMETHODCALLTYPE fake_trust(IInspectable *self, TrustLevel *level)
{ (void)self; (void)level; return E_NOTIMPL; }
static IInspectableVtbl fake_vtable={fake_query,fake_addref,fake_release,fake_iids,fake_name,fake_trust};

static int fail_apply, fail_remove, reenter_apply, reenter_remove;
static HRESULT nested_result;
static win16_xaml_button_set *current_set;
static IInspectable *unknown_owner;
static IVisualTreeService *fake_service=(IVisualTreeService *)(UINT_PTR)1;
static IXamlDiagnostics *fake_diagnostics=(IXamlDiagnostics *)(UINT_PTR)1;
HRESULT win16_xaml_element_owner(IInspectable *element,HWND *owner)
{ if(!element || !owner)return E_INVALIDARG; *owner=NULL; if(element==unknown_owner)return E_NOINTERFACE; *owner=(HWND)(UINT_PTR)element; return S_OK; }

HRESULT win16_xaml_button_apply(IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                               IInspectable *button, win16_xaml_button_style *state)
{
    if (reenter_apply) {
        win16_xaml_button_style *nested=NULL;
        nested_result=win16_xaml_button_set_add(current_set,service,diagnostics,button,&nested);
    }
    if (fail_apply) return E_FAIL;
    IInspectable_AddRef(button); state->button=button; state->thread_id=GetCurrentThreadId();
    state->handle=(InstanceHandle)(UINT_PTR)button;
    return S_OK;
}
HRESULT win16_xaml_button_remove(IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                                win16_xaml_button_style *state)
{
    (void)service; (void)diagnostics;
    if (reenter_remove)
        nested_result=win16_xaml_button_set_remove_handle(current_set,service,diagnostics,state->handle);
    if (fail_remove) return HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH);
    IInspectable_Release(state->button); *state=(win16_xaml_button_style){0}; return S_OK;
}
void win16_xaml_button_discard(win16_xaml_button_style *state)
{
    if (state->button) IInspectable_Release(state->button);
    *state=(win16_xaml_button_style){0};
}
static DWORD WINAPI wrong_thread(void *arg)
{ return (DWORD)win16_xaml_button_set_discard(arg); }

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr,"line %d: %s\n",__LINE__,#expr); return 1; } } while (0)
int main(void)
{
    fake_button a={{&fake_vtable},1}, b={{&fake_vtable},1};
    win16_xaml_button_set set={0}; current_set=&set;
    win16_xaml_button_style *one=NULL,*two=NULL,*duplicate=NULL;
    fail_apply=1;
    CHECK(FAILED(win16_xaml_button_set_add(&set,fake_service,fake_diagnostics,&a.iface,&one)));
    CHECK(!set.first && !set.thread_id && a.references==1);
    fail_apply=0; reenter_apply=1;
    CHECK(win16_xaml_button_set_add(&set,fake_service,fake_diagnostics,&a.iface,&one)==S_OK);
    CHECK(nested_result==HRESULT_FROM_WIN32(ERROR_BUSY)); reenter_apply=0;
    CHECK(win16_xaml_button_set_add(&set,fake_service,fake_diagnostics,&a.iface,&duplicate)==S_FALSE && duplicate==one);
    CHECK(win16_xaml_button_set_add(&set,fake_service,fake_diagnostics,&b.iface,&two)==S_OK && two!=one);
    HANDLE worker=CreateThread(NULL,0,wrong_thread,&set,0,NULL); CHECK(worker!=NULL);
    CHECK(WaitForSingleObject(worker,2000)==WAIT_OBJECT_0);
    DWORD result=0; CHECK(GetExitCodeThread(worker,&result)); CloseHandle(worker);
    CHECK(result==(DWORD)RPC_E_WRONG_THREAD);
    fail_remove=1;
    CHECK(win16_xaml_button_set_remove(&set,fake_service,fake_diagnostics,one)==HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH));
    CHECK(win16_xaml_button_set_add(&set,fake_service,fake_diagnostics,&a.iface,&duplicate)==S_FALSE && duplicate==one);
    CHECK(win16_xaml_button_set_remove_all(&set,fake_service,fake_diagnostics)==HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH));
    CHECK(a.references==3 && b.references==3);
    fail_remove=0;
    CHECK(win16_xaml_button_set_remove_handle(&set,fake_service,fake_diagnostics,(InstanceHandle)1)==S_FALSE);
    reenter_remove=1;
    CHECK(win16_xaml_button_set_remove_window(&set,fake_service,fake_diagnostics,(HWND)(UINT_PTR)&b.iface)==S_OK);
    CHECK(nested_result==HRESULT_FROM_WIN32(ERROR_BUSY));
    reenter_remove=0;
    CHECK(b.references==1 && a.references==3);
    CHECK(win16_xaml_button_set_remove_all(&set,fake_service,fake_diagnostics)==S_OK);
    CHECK(!set.first && !set.thread_id && a.references==1 && b.references==1);
    CHECK(win16_xaml_button_set_add(&set,fake_service,fake_diagnostics,&a.iface,&one)==S_OK);
    CHECK(win16_xaml_button_set_add(&set,fake_service,fake_diagnostics,&b.iface,&two)==S_OK);
    unknown_owner=&b.iface;
    CHECK(win16_xaml_button_set_remove_window(&set,fake_service,fake_diagnostics,(HWND)(UINT_PTR)&a.iface)==S_FALSE);
    CHECK(a.references==1 && b.references==3);
    unknown_owner=NULL;
    CHECK(win16_xaml_button_set_remove_all(&set,fake_service,fake_diagnostics)==S_OK);
    CHECK(a.references==1 && b.references==1);
    CHECK(win16_xaml_button_set_discard(&set)==S_OK);
    return 0;
}
