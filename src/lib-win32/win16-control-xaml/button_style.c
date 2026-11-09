#define COBJMACROS
#include "button_style.h"
#include "button_template.h"
#include "checkbox_template.h"
#include "radio_template.h"
#include <oleauto.h>
#include <roapi.h>
#include <winstring.h>
#include <wchar.h>

typedef struct reader reader;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(reader *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(reader *);
    ULONG (STDMETHODCALLTYPE *release)(reader *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(reader *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(reader *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(reader *, TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *load)(reader *, HSTRING, IInspectable **);
} reader_vtable;
struct reader { const reader_vtable *vtable; };

typedef struct element element;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(element *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(element *);
    ULONG (STDMETHODCALLTYPE *release)(element *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(element *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(element *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(element *, TrustLevel *);
    void (STDMETHODCALLTYPE *unused[46])(void);
    HRESULT (STDMETHODCALLTYPE *get_style)(element *, IInspectable **);
} element_vtable;
struct element { const element_vtable *vtable; };

static const IID reader_iid = {0x82a4cd9e,0x435e,0x5aeb,{0x8c,0x4f,0x30,0x0c,0xec,0xe4,0x5c,0xae}};
static const IID element_iid = {0xfe08f13d,0xdc6a,0x5495,{0xad,0x44,0xc2,0xd8,0xd2,0x18,0x63,0xb0}};

HRESULT win16_xaml_button_source(IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                                 IInspectable *button, unsigned int *source, unsigned int *index)
{
    if (!service || !diagnostics || !button || !source || !index) return E_INVALIDARG;
    InstanceHandle handle=0;
    PropertyChainSource *sources=NULL;
    PropertyChainValue *values=NULL;
    unsigned int source_count=0, value_count=0, matches=0;
    *source=0; *index=0;
    HRESULT hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,button,&handle);
    if (SUCCEEDED(hr)) hr=IVisualTreeService_GetPropertyValuesChain(service,handle,&source_count,&sources,&value_count,&values);
    if (SUCCEEDED(hr)) {
        for (unsigned int i=0;i<value_count;i++) {
            if (values[i].PropertyName && !wcscmp(values[i].PropertyName,L"Style") && !values[i].Overridden) {
                if (values[i].PropertyChainIndex>=source_count) { hr=E_UNEXPECTED; break; }
                ++matches; *index=values[i].Index;
                *source=(unsigned int)sources[values[i].PropertyChainIndex].Source;
            }
        }
        if (SUCCEEDED(hr) && matches!=1) hr=HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }
    for (unsigned int i=0;i<source_count;i++) {
        SysFreeString(sources[i].TargetType); SysFreeString(sources[i].Name);
        SysFreeString(sources[i].SrcInfo.FileName); SysFreeString(sources[i].SrcInfo.Hash);
    }
    for (unsigned int i=0;i<value_count;i++) {
        SysFreeString(values[i].Type); SysFreeString(values[i].DeclaringType); SysFreeString(values[i].ValueType);
        SysFreeString(values[i].ItemType); SysFreeString(values[i].Value); SysFreeString(values[i].PropertyName);
    }
    CoTaskMemFree(sources); CoTaskMemFree(values);
    return hr;
}

HRESULT win16_xaml_button_apply(IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                                IInspectable *button, win16_xaml_button_style *state)
{
    static const wchar_t name_value[]=L"Microsoft.UI.Xaml.Markup.XamlReader";
    if (!service || !diagnostics || !button || !state || state->button || state->installed_style) return E_INVALIDARG;
    unsigned int source=0,index=0;
    HRESULT hr=win16_xaml_button_source(service,diagnostics,button,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    HSTRING name=NULL,text=NULL;
    reader *factory=NULL;
    IInspectable *style=NULL;
    InstanceHandle target=0,value=0;
    HSTRING runtime_name=NULL;
    const wchar_t *markup=NULL;
    if (SUCCEEDED(hr)) hr=IInspectable_GetRuntimeClassName(button,&runtime_name);
    if (SUCCEEDED(hr) && runtime_name) {
        const wchar_t *runtime=WindowsGetStringRawBuffer(runtime_name,NULL);
        if (!wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.Button")) markup=win16_xaml_button_template();
        else if (!wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.CheckBox")) markup=win16_xaml_checkbox_template();
        else if (!wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.RadioButton")) markup=win16_xaml_radio_template();
    }
    if (SUCCEEDED(hr) && !markup) hr=HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    if (runtime_name) WindowsDeleteString(runtime_name);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(name_value,(UINT32)(ARRAYSIZE(name_value)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&reader_iid,(void **)&factory);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(markup,(UINT32)lstrlenW(markup),&text);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,text,&style);
    if (SUCCEEDED(hr)) hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,button,&target);
    if (SUCCEEDED(hr)) hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,style,&value);
    if (SUCCEEDED(hr)) hr=IVisualTreeService_SetProperty(service,target,value,index);
    if (SUCCEEDED(hr)) {
        IInspectable_AddRef(button);
        state->button=button; state->installed_style=style; style=NULL;
        state->property_index=index; state->thread_id=GetCurrentThreadId(); state->handle=target;
    }
    if (style) IInspectable_Release(style);
    if (factory) factory->vtable->release(factory);
    if (text) WindowsDeleteString(text);
    if (name) WindowsDeleteString(name);
    return hr;
}

HRESULT win16_xaml_button_remove(IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                                 win16_xaml_button_style *state)
{
    if (!service || !diagnostics || !state || !state->button || !state->installed_style) return E_INVALIDARG;
    if (state->thread_id!=GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
    element *owner=NULL;
    IInspectable *current=NULL;
    IUnknown *a=NULL,*b=NULL;
    HRESULT hr=IInspectable_QueryInterface(state->button,&element_iid,(void **)&owner);
    if (SUCCEEDED(hr)) hr=owner->vtable->get_style(owner,&current);
    if (SUCCEEDED(hr) && !current) hr=HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(current,&IID_IUnknown,(void **)&a);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(state->installed_style,&IID_IUnknown,(void **)&b);
    if (SUCCEEDED(hr) && a!=b) hr=HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH);
    InstanceHandle target=0;
    if (SUCCEEDED(hr)) hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,state->button,&target);
    if (SUCCEEDED(hr)) hr=IVisualTreeService_ClearProperty(service,target,state->property_index);
    if (a) IUnknown_Release(a);
    if (b) IUnknown_Release(b);
    if (current) IInspectable_Release(current);
    if (owner) owner->vtable->release(owner);
    if (SUCCEEDED(hr)) win16_xaml_button_discard(state);
    return hr;
}

void win16_xaml_button_discard(win16_xaml_button_style *state)
{
    if (!state) return;
    if (state->button) IInspectable_Release(state->button);
    if (state->installed_style) IInspectable_Release(state->installed_style);
    *state=(win16_xaml_button_style){0};
}
