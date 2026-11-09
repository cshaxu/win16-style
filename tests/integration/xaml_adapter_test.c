#define COBJMACROS
#include "lib-win32/win16-control-xaml/adapter.h"
#include <stdio.h>

static HRESULT result;
static unsigned calls;
static win16_xaml_adapter *stop_during_resolve;
static HRESULT STDMETHODCALLTYPE resolve(IXamlDiagnostics *self, InstanceHandle handle, IInspectable **object)
{
    (void)self;
    if (handle!=42) return E_INVALIDARG;
    ++calls;
    if (stop_during_resolve) (void)win16_xaml_adapter_stop(stop_during_resolve);
    *object=NULL;
    return result;
}
static IXamlDiagnosticsVtbl vtable={.GetIInspectableFromHandle=resolve};
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr,"line %d: %s\n",__LINE__,#expr); return 1; } } while (0)
int main(void)
{
    IXamlDiagnostics diagnostics={&vtable};
    win16_xaml_adapter adapter={0};
    adapter.service=(IVisualTreeService *)(UINT_PTR)1;
    adapter.diagnostics=&diagnostics;
    adapter.connected=TRUE;
    ParentChildRelation relation={0}; relation.Parent=42;
    VisualElement element={0};
    IInspectable *applied=(IInspectable *)(UINT_PTR)1;
    result=HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    CHECK(win16_xaml_adapter_change(&adapter,relation,element,Add,&applied)==S_FALSE);
    CHECK(!applied && calls==1 && !adapter.session.service);
    result=E_ACCESSDENIED;
    CHECK(win16_xaml_adapter_change(&adapter,relation,element,Add,NULL)==E_ACCESSDENIED);
    CHECK(calls==2 && !adapter.session.service);
    CHECK(win16_xaml_adapter_change(&adapter,relation,element,Remove,NULL)==S_FALSE && calls==2);
    stop_during_resolve=&adapter;
    CHECK(win16_xaml_adapter_change(&adapter,relation,element,Add,NULL)==S_FALSE && calls==3);
    stop_during_resolve=NULL;
    CHECK(win16_xaml_adapter_change(&adapter,relation,element,Add,NULL)==S_FALSE && calls==3);
    return 0;
}
