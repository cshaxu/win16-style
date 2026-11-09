#define COBJMACROS
#include "lib-win32/win16-control-xaml/session.h"
#include "lib-win32/win16-control-xaml/visual_tree.h"
#include <stdio.h>
#include <winstring.h>

typedef struct fake { IInspectable iface; int kind; LONG references; } fake;
static HRESULT STDMETHODCALLTYPE query(IInspectable *p,REFIID iid,void **out)
{ (void)iid; if (!out) return E_POINTER; *out=p; IInspectable_AddRef(p); return S_OK; }
static ULONG STDMETHODCALLTYPE add(IInspectable *p) { return (ULONG)InterlockedIncrement(&((fake *)p)->references); }
static ULONG STDMETHODCALLTYPE release(IInspectable *p) { return (ULONG)InterlockedDecrement(&((fake *)p)->references); }
static HRESULT STDMETHODCALLTYPE iids(IInspectable *p,ULONG *n,IID **ids)
{ (void)p; (void)n; (void)ids; return E_NOTIMPL; }
static HRESULT STDMETHODCALLTYPE name(IInspectable *p,HSTRING *out)
{
    int kind=((fake *)p)->kind;
    const WCHAR *s=kind==1 ? L"Microsoft.UI.Xaml.Controls.Button" :
        kind==2 ? L"Microsoft.UI.Xaml.Controls.CheckBox" :
        kind==3 ? L"Microsoft.UI.Xaml.Controls.RadioButton" : L"Microsoft.UI.Xaml.Controls.Grid";
    return WindowsCreateString(s,(UINT32)lstrlenW(s),out);
}
static HRESULT STDMETHODCALLTYPE trust(IInspectable *p,TrustLevel *level)
{ (void)p; (void)level; return E_NOTIMPL; }
static IInspectableVtbl vtable={query,add,release,iids,name,trust};
static BOOL unsupported=TRUE;
static win16_xaml_button_style mock_style;
HRESULT win16_xaml_button_set_add(win16_xaml_button_set *set,IVisualTreeService *service,
                                  IXamlDiagnostics *diagnostics,IInspectable *button,win16_xaml_button_style **state)
{
    (void)service; (void)diagnostics; (void)button;
    if (unsupported) return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    set->first=(win16_xaml_button_entry *)(UINT_PTR)1; *state=&mock_style; return S_OK;
}
HRESULT win16_xaml_button_set_remove_all(win16_xaml_button_set *set,IVisualTreeService *service,IXamlDiagnostics *diagnostics)
{ (void)service; (void)diagnostics; set->first=NULL; return S_OK; }
HRESULT win16_xaml_visit_ancestors(IInspectable *object,win16_xaml_ancestor_visitor visitor,void *context)
{ (void)visitor(object,0,context); return S_OK; }
HRESULT win16_xaml_visit_descendants(IInspectable *object,win16_xaml_ancestor_visitor visitor,void *context)
{ (void)visitor(object,0,context); return S_OK; }
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
int main(void)
{
    win16_xaml_session session={0};
    fake grid={{&vtable},0,1},button={{&vtable},1,1},checkbox={{&vtable},2,1},radio={{&vtable},3,1};
    CHECK(win16_xaml_session_init(&session,(IVisualTreeService *)(UINT_PTR)1,(IXamlDiagnostics *)(UINT_PTR)1)==S_OK);
    CHECK(!session.thread_id);
    CHECK(win16_xaml_session_observe(&session,&grid.iface,NULL)==S_FALSE && !session.thread_id);
    CHECK(win16_xaml_session_observe(&session,&checkbox.iface,NULL)==S_FALSE && !session.thread_id);
    CHECK(win16_xaml_session_observe(&session,&radio.iface,NULL)==S_FALSE && !session.thread_id);
    CHECK(win16_xaml_session_observe(&session,&button.iface,NULL)==S_FALSE && !session.thread_id);
    unsupported=FALSE;
    CHECK(win16_xaml_session_observe(&session,&button.iface,NULL)==S_OK && session.thread_id==GetCurrentThreadId());
    CHECK(win16_xaml_session_stop(&session)==S_OK && !session.service && !session.thread_id);
    CHECK(win16_xaml_session_init(&session,(IVisualTreeService *)(UINT_PTR)1,(IXamlDiagnostics *)(UINT_PTR)1)==S_OK);
    CHECK(win16_xaml_session_observe(&session,&checkbox.iface,NULL)==S_OK && session.thread_id==GetCurrentThreadId());
    CHECK(win16_xaml_session_stop(&session)==S_OK && !session.service);
    CHECK(win16_xaml_session_init(&session,(IVisualTreeService *)(UINT_PTR)1,(IXamlDiagnostics *)(UINT_PTR)1)==S_OK);
    CHECK(win16_xaml_session_observe(&session,&radio.iface,NULL)==S_OK);
    CHECK(win16_xaml_session_stop(&session)==S_OK && !session.service);
    CHECK(grid.references==1 && button.references==1 && checkbox.references==1 && radio.references==1);
    return 0;
}
