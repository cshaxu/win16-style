#define COBJMACROS
#include "owner.h"
#include <stdint.h>

typedef struct root_access root_access;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(root_access *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(root_access *);
    ULONG (STDMETHODCALLTYPE *release)(root_access *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(root_access *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(root_access *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(root_access *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[103])(void);
    HRESULT (STDMETHODCALLTYPE *get_root)(root_access *,IInspectable **);
} root_access_vtable;
struct root_access { const root_access_vtable *vtable; };
typedef struct root_environment root_environment;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(root_environment *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(root_environment *);
    ULONG (STDMETHODCALLTYPE *release)(root_environment *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(root_environment *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(root_environment *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(root_environment *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_environment)(root_environment *,IInspectable **);
} root_environment_vtable;
struct root_environment { const root_environment_vtable *vtable; };
typedef struct island_environment island_environment;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(island_environment *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(island_environment *);
    ULONG (STDMETHODCALLTYPE *release)(island_environment *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(island_environment *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(island_environment *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(island_environment *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_window_id)(island_environment *,UINT64 *);
} island_environment_vtable;
struct island_environment { const island_environment_vtable *vtable; };
static const IID ui_iid={0xc3c01020,0x320c,0x5cf6,{0x9d,0x24,0xd3,0x96,0xbb,0xfa,0x4d,0x8b}};
static const IID root_iid={0xbdee0f42,0x71cb,0x50c5,{0x82,0x9b,0x46,0x14,0xd9,0x8c,0x57,0x94}};
static const IID island_iid={0xc334236d,0xda88,0x566d,{0x81,0x1d,0x74,0xae,0xf2,0xeb,0xa9,0x78}};

HRESULT win16_xaml_element_owner(IInspectable *element,HWND *owner)
{
    if (!owner) return E_INVALIDARG;
    *owner=NULL;
    if (!element) return E_INVALIDARG;
    root_access *ui=NULL; IInspectable *root=NULL,*environment=NULL;
    root_environment *root2=NULL; island_environment *island=NULL; UINT64 id=0;
    HMODULE module=GetModuleHandleW(L"Microsoft.Internal.FrameworkUdk.dll");
    HRESULT (WINAPI *resolve)(UINT64,HWND *)=(void *)(uintptr_t)(module ? GetProcAddress(module,"Windowing_GetWindowFromWindowId") : NULL);
    HRESULT hr=resolve ? IInspectable_QueryInterface(element,&ui_iid,(void **)&ui) : HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
    if (SUCCEEDED(hr)) hr=ui->vtable->get_root(ui,&root);
    if (SUCCEEDED(hr) && !root) hr=HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&root_iid,(void **)&root2);
    if (SUCCEEDED(hr)) hr=root2->vtable->get_environment(root2,&environment);
    if (SUCCEEDED(hr) && !environment) hr=HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(environment,&island_iid,(void **)&island);
    if (SUCCEEDED(hr)) hr=island->vtable->get_window_id(island,&id);
    if (SUCCEEDED(hr)) hr=resolve(id,owner);
    if (island) island->vtable->release(island);
    if (environment) IInspectable_Release(environment);
    if (root2) root2->vtable->release(root2);
    if (root) IInspectable_Release(root);
    if (ui) ui->vtable->release(ui);
    return hr;
}
