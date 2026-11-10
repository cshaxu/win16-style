/* Test-only: runs on the owning UI thread of a self-created target. */
#define COBJMACROS
#define INITGUID
#include "lib-win32/win16-control-xaml/adapter.h"
#include "lib-win32/win16-control-xaml/visual_tree.h"
#include "lib-win32/win16-control-xaml/owner.h"
#include <stdint.h>
#include <roapi.h>
#include <winstring.h>
#include <oleauto.h>
#include <wchar.h>

typedef struct observed_entry {
    void *next;
    IUnknown *identity;
    BOOL installing;
    win16_xaml_button_style style;
} observed_entry;
static IVisualTreeService *service;
static IXamlDiagnostics *diagnostics;
static IInspectable *button;
static IInspectable *other_button;
typedef struct test_window test_window;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_window *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_window *);
    ULONG (STDMETHODCALLTYPE *release)(test_window *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_window *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_window *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_window *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused0[2])(void);
    HRESULT (STDMETHODCALLTYPE *get_content)(test_window *,IInspectable **);
    HRESULT (STDMETHODCALLTYPE *put_content)(test_window *,IInspectable *);
    void (STDMETHODCALLTYPE *unused1[16])(void);
    HRESULT (STDMETHODCALLTYPE *activate)(test_window *);
    HRESULT (STDMETHODCALLTYPE *close)(test_window *);
} test_window_vtable;
struct test_window { const test_window_vtable *vtable; };
static const IID window_iid={0x61f0ec79,0x5d52,0x56b5,{0x86,0xfb,0x40,0xfa,0x4a,0xf2,0x88,0xb0}};
typedef struct window_native window_native;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(window_native *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(window_native *);
    ULONG (STDMETHODCALLTYPE *release)(window_native *);
    HRESULT (STDMETHODCALLTYPE *get_hwnd)(window_native *,HWND *);
} window_native_vtable;
struct window_native { const window_native_vtable *vtable; };
static const IID window_native_iid={0xeecdbf0e,0xbae9,0x4cb6,{0xa6,0x8e,0x95,0x98,0xe1,0xcb,0x57,0xbb}};
static const IID ui_iid={0xc3c01020,0x320c,0x5cf6,{0x9d,0x24,0xd3,0x96,0xbb,0xfa,0x4d,0x8b}};
typedef struct input_element input_element;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(input_element *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(input_element *);
    ULONG (STDMETHODCALLTYPE *release)(input_element *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(input_element *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(input_element *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(input_element *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[214])(void);
    HRESULT (STDMETHODCALLTYPE *update_layout)(input_element *);
    void (STDMETHODCALLTYPE *unused_focus[5])(void);
    HRESULT (STDMETHODCALLTYPE *focus)(input_element *,INT32,boolean *);
} input_element_vtable;
struct input_element { const input_element_vtable *vtable; };
typedef struct input_textbox input_textbox;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(input_textbox *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(input_textbox *);
    ULONG (STDMETHODCALLTYPE *release)(input_textbox *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(input_textbox *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(input_textbox *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(input_textbox *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_text)(input_textbox *,HSTRING *);
} input_textbox_vtable;
struct input_textbox { const input_textbox_vtable *vtable; };
static const IID input_textbox_iid={0x873af7c2,0xab89,0x5d76,{0x8d,0xbe,0x3d,0x63,0x25,0x66,0x9d,0xf5}};
typedef struct xaml_reader xaml_reader;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(xaml_reader *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(xaml_reader *);
    ULONG (STDMETHODCALLTYPE *release)(xaml_reader *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(xaml_reader *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(xaml_reader *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(xaml_reader *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *load)(xaml_reader *,HSTRING,IInspectable **);
} xaml_reader_vtable;
struct xaml_reader { const xaml_reader_vtable *vtable; };
static const IID xaml_reader_iid={0x82a4cd9e,0x435e,0x5aeb,{0x8c,0x4f,0x30,0x0c,0xec,0xe4,0x5c,0xae}};
typedef struct combo_control combo_control;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(combo_control *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(combo_control *);
    ULONG (STDMETHODCALLTYPE *release)(combo_control *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(combo_control *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(combo_control *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(combo_control *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused)(void);
    HRESULT (STDMETHODCALLTYPE *put_open)(combo_control *,boolean);
} combo_control_vtable;
struct combo_control { const combo_control_vtable *vtable; };
static const IID combo_control_iid={0xc77da58b,0x4fd7,0x51e0,{0xa4,0x31,0xf8,0x46,0x58,0xa8,0x3e,0x9e}};
typedef struct item_mapping item_mapping;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(item_mapping *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(item_mapping *);
    ULONG (STDMETHODCALLTYPE *release)(item_mapping *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(item_mapping *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(item_mapping *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(item_mapping *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[3])(void);
    HRESULT (STDMETHODCALLTYPE *container_from_index)(item_mapping *,INT32,IInspectable **);
} item_mapping_vtable;
struct item_mapping { const item_mapping_vtable *vtable; };
static const IID item_mapping_iid={0xb3e03a32,0xea83,0x5626,{0xbf,0xda,0xd3,0x27,0xa1,0xc7,0x55,0x0c}};
typedef struct test_control test_control;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_control *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_control *);
    ULONG (STDMETHODCALLTYPE *release)(test_control *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_control *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_control *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_control *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[55])(void);
    HRESULT (STDMETHODCALLTYPE *apply_template)(test_control *,boolean *);
} test_control_vtable;
struct test_control { const test_control_vtable *vtable; };
static const IID control_iid={0x857d6e8a,0xd45a,0x5c69,{0xa9,0x9c,0xbf,0x6a,0x5c,0x54,0xfb,0x38}};
static test_window *fixture_window;
static unsigned int fixture_phase;
static unsigned int fixture_nodes,fixture_max_depth,fixture_entries;
typedef struct test_panel test_panel;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_panel *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_panel *);
    ULONG (STDMETHODCALLTYPE *release)(test_panel *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_panel *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_panel *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_panel *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_children)(test_panel *,IInspectable **);
} test_panel_vtable;
struct test_panel { const test_panel_vtable *vtable; };
static const IID panel_iid={0x27a1b418,0x56f3,0x525e,{0xb8,0x83,0xce,0xfe,0xd9,0x05,0xee,0xd3}};
static BOOL find_panel(IInspectable *object,unsigned int depth,void *context)
{
    ++fixture_nodes;
    if (depth>fixture_max_depth) fixture_max_depth=depth;
    return FAILED(IInspectable_QueryInterface(object,&panel_iid,context));
}
static BOOL children_are_collection(InstanceHandle handle)
{
    PropertyChainSource *sources=NULL; PropertyChainValue *values=NULL;
    unsigned int source_count=0,value_count=0; BOOL found=FALSE;
    HRESULT hr=IVisualTreeService_GetPropertyValuesChain(service,handle,&source_count,&sources,&value_count,&values);
    if (SUCCEEDED(hr)) for (unsigned int i=0;i<value_count;i++)
        if (values[i].PropertyName && !wcscmp(values[i].PropertyName,L"Children") &&
            !values[i].Overridden && (values[i].MetadataBits & IsValueCollection)) found=TRUE;
    for (unsigned int i=0;i<source_count;i++) {
        SysFreeString(sources[i].TargetType); SysFreeString(sources[i].Name);
        SysFreeString(sources[i].SrcInfo.FileName); SysFreeString(sources[i].SrcInfo.Hash);
    }
    for (unsigned int i=0;i<value_count;i++) {
        SysFreeString(values[i].Type); SysFreeString(values[i].DeclaringType); SysFreeString(values[i].ValueType);
        SysFreeString(values[i].ItemType); SysFreeString(values[i].Value); SysFreeString(values[i].PropertyName);
    }
    CoTaskMemFree(sources); CoTaskMemFree(values); return found;
}
static void clear(void)
{
    if (fixture_window) { (void)fixture_window->vtable->close(fixture_window); fixture_window->vtable->release(fixture_window); }
    fixture_window=NULL;
    if (button) IInspectable_Release(button);
    if (other_button) IInspectable_Release(other_button);
    if (diagnostics) IXamlDiagnostics_Release(diagnostics);
    if (service) IVisualTreeService_Release(service);
    button=NULL; other_button=NULL; diagnostics=NULL; service=NULL;
}
static HRESULT create_checkbox(win16_xaml_adapter *adapter,int kind)
{
    if (adapter && (!adapter->service || !adapter->diagnostics)) return E_INVALIDARG;
    HMODULE pin=NULL;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                            (PCWSTR)(uintptr_t)create_checkbox,&pin)) return HRESULT_FROM_WIN32(GetLastError());
    if (adapter) {
        service=adapter->service; diagnostics=adapter->diagnostics;
        IVisualTreeService_AddRef(service); IXamlDiagnostics_AddRef(diagnostics);
    }
    HSTRING name=NULL; IInspectable *object=NULL,*element=NULL; test_control *control=NULL;
    boolean applied=0;
    const WCHAR *class_name=kind==6 ? L"Microsoft.UI.Xaml.Controls.ProgressBar" : kind==5 ? L"Microsoft.UI.Xaml.Controls.ListBox" : kind==4 ? L"Microsoft.UI.Xaml.Controls.Primitives.ScrollBar" : kind==3 ? L"Microsoft.UI.Xaml.Controls.ComboBox" : kind==2 ? L"Microsoft.UI.Xaml.Controls.TextBox" :
        kind==1 ? L"Microsoft.UI.Xaml.Controls.RadioButton" : L"Microsoft.UI.Xaml.Controls.CheckBox";
    HRESULT hr=S_OK;
    if (kind==3 || kind==5 || kind==6) {
        static const WCHAR reader_name[]=L"Microsoft.UI.Xaml.Markup.XamlReader";
        static const WCHAR markup[]=L"<ComboBox xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml'><x:String>First</x:String><x:String>Second</x:String></ComboBox>";
        static const WCHAR list_markup[]=L"<ListBox xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml'><x:String>First</x:String><x:String>Second</x:String></ListBox>";
        static const WCHAR progress_markup[]=L"<ProgressBar xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation'/>";
        xaml_reader *reader=NULL; HSTRING text=NULL;
        hr=WindowsCreateString(reader_name,(UINT32)(ARRAYSIZE(reader_name)-1),&name);
        if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&xaml_reader_iid,(void **)&reader);
        const WCHAR *chosen=kind==6 ? progress_markup : kind==5 ? list_markup : markup;
        if (SUCCEEDED(hr)) hr=WindowsCreateString(chosen,(UINT32)lstrlenW(chosen),&text);
        if (SUCCEEDED(hr)) hr=reader->vtable->load(reader,text,&button);
        if (reader) reader->vtable->release(reader);
        if (text) WindowsDeleteString(text);
    } else {
        hr=WindowsCreateString(class_name,(UINT32)lstrlenW(class_name),&name);
        if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&button);
    }
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&object);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&window_iid,(void **)&fixture_window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&ui_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=fixture_window->vtable->put_content(fixture_window,element);
    if (SUCCEEDED(hr)) hr=fixture_window->vtable->activate(fixture_window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&control_iid,(void **)&control);
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (control) control->vtable->release(control);
    if (element) IInspectable_Release(element);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    if (FAILED(hr)) clear();
    return hr;
}
static HRESULT create_checkbox_in_tree(win16_xaml_adapter *adapter)
{
    observed_entry *entry=adapter ? (observed_entry *)adapter->session.buttons.first : NULL;
    if (!adapter || !adapter->service || !adapter->diagnostics || !entry || !entry->style.button ||
        adapter->session.thread_id!=GetCurrentThreadId()) return E_INVALIDARG;
    HMODULE pin=NULL;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                            (PCWSTR)(uintptr_t)create_checkbox_in_tree,&pin)) return HRESULT_FROM_WIN32(GetLastError());
    service=adapter->service; diagnostics=adapter->diagnostics;
    IVisualTreeService_AddRef(service); IXamlDiagnostics_AddRef(diagnostics);
    test_panel *panel=NULL; IInspectable *children=NULL; HSTRING name=NULL;
    InstanceHandle panel_handle=0,collection_handle=0,child_handle=0; unsigned int count=0;
    fixture_phase=1;
    fixture_nodes=fixture_max_depth=fixture_entries=0;
    HRESULT hr=S_OK;
    for (;entry && !panel;entry=(observed_entry *)entry->next) {
        ++fixture_entries;
        if (!entry->installing && entry->style.button)
            hr=win16_xaml_visit_ancestors(entry->style.button,find_panel,&panel);
        if (FAILED(hr)) break;
    }
    if (SUCCEEDED(hr) && !panel) hr=HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    if (SUCCEEDED(hr)) fixture_phase=2;
    if (SUCCEEDED(hr)) hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,(IInspectable *)panel,&panel_handle);
    if (SUCCEEDED(hr)) fixture_phase=3;
    if (SUCCEEDED(hr) && !children_are_collection(panel_handle)) hr=HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    if (SUCCEEDED(hr)) fixture_phase=4;
    if (SUCCEEDED(hr)) hr=panel->vtable->get_children(panel,&children);
    if (SUCCEEDED(hr)) fixture_phase=5;
    if (SUCCEEDED(hr)) hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,children,&collection_handle);
    if (SUCCEEDED(hr)) fixture_phase=6;
    if (SUCCEEDED(hr)) hr=IVisualTreeService_GetCollectionCount(service,collection_handle,&count);
    if (SUCCEEDED(hr)) fixture_phase=7;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Controls.CheckBox",35,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&button);
    if (SUCCEEDED(hr)) fixture_phase=8;
    if (SUCCEEDED(hr)) hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,button,&child_handle);
    if (SUCCEEDED(hr)) fixture_phase=9;
    if (SUCCEEDED(hr)) hr=IVisualTreeService_AddChild(service,collection_handle,child_handle,count);
    if (SUCCEEDED(hr)) fixture_phase=10;
    if (name) WindowsDeleteString(name);
    if (children) IInspectable_Release(children);
    if (panel) panel->vtable->release(panel);
    if (FAILED(hr)) clear();
    return hr;
}
__declspec(dllexport) LRESULT CALLBACK XamlPropertyProbe(int code,WPARAM wp,LPARAM lp)
{
    const CWPSTRUCT *m=(const CWPSTRUCT *)lp;
    if (code>=0 && m && m->message==RegisterWindowMessageW(L"Win16Xaml.Test.Property")) {
        HRESULT hr=E_INVALIDARG; unsigned int source=0,index=0;
        if (m->wParam==1) {
            clear();
            win16_xaml_adapter *adapter=(win16_xaml_adapter *)m->lParam;
            observed_entry *entry=adapter ? (observed_entry *)adapter->session.buttons.first : NULL;
            if (adapter && adapter->session.thread_id==GetCurrentThreadId() && entry &&
                !entry->installing && entry->style.button && adapter->service && adapter->diagnostics) {
                HMODULE pin=NULL;
                if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                                       (PCWSTR)(uintptr_t)XamlPropertyProbe,&pin)) {
                    service=adapter->service; diagnostics=adapter->diagnostics; button=entry->style.button;
                    IVisualTreeService_AddRef(service); IXamlDiagnostics_AddRef(diagnostics); IInspectable_AddRef(button);
                    hr=win16_xaml_button_source(service,diagnostics,button,&source,&index);
                }
            }
        } else if (m->wParam==3 || m->wParam==5 || m->wParam==7 || m->wParam==17 || m->wParam==18 || m->wParam==21 || m->wParam==24 || m->wParam==27 || m->wParam==29) {
            clear();
            hr=m->wParam==7 ? create_checkbox(NULL,0) : m->wParam==3 || m->wParam==17 || m->wParam==18 || m->wParam==21 || m->wParam==24 || m->wParam==27 || m->wParam==29
                ? create_checkbox((win16_xaml_adapter *)m->lParam,m->wParam==29 ? 6 : m->wParam==27 ? 5 : m->wParam==24 ? 4 : m->wParam==21 ? 3 : m->wParam==18 ? 2 : m->wParam==17 ? 1 : 0)
                : create_checkbox_in_tree((win16_xaml_adapter *)m->lParam);
        } else if (m->wParam==4 && button) {
            win16_xaml_adapter *adapter=(win16_xaml_adapter *)m->lParam;
            if (!service && adapter && adapter->service && adapter->diagnostics) {
                service=adapter->service; diagnostics=adapter->diagnostics;
                IVisualTreeService_AddRef(service); IXamlDiagnostics_AddRef(diagnostics);
            }
            hr=win16_xaml_button_source(service,diagnostics,button,&source,&index);
        } else if (m->wParam==8) {
            win16_xaml_adapter *adapter=(win16_xaml_adapter *)m->lParam;
            hr=adapter && adapter->service ? S_OK : S_FALSE;
            source=hr==S_OK ? 1 : 0;
        } else if (m->wParam==9) {
            HMODULE product=GetModuleHandleW(sizeof(void *)==8 ? L"ntw16s64.dll" : L"ntw16s32.dll");
            BOOL (__cdecl *attached)(HWND)=(void *)(uintptr_t)(product ? GetProcAddress(product,"Ntw16sWindowAttached") : NULL);
            if (attached) { source=attached(m->hwnd) ? 1 : 0; hr=S_OK; }
        } else if (m->wParam==10 && fixture_window) {
            hr=fixture_window->vtable->close(fixture_window);
            fixture_window->vtable->release(fixture_window); fixture_window=NULL;
        } else if (m->wParam==11) {
            HWND bridge=FindWindowExW(m->hwnd,NULL,L"Microsoft.UI.Content.DesktopChildSiteBridge",NULL);
            if (!bridge) bridge=FindWindowExW(m->hwnd,NULL,L"Windows.UI.Composition.DesktopWindowContentBridge",NULL);
            hr=bridge && DestroyWindow(bridge) ? S_OK : HRESULT_FROM_WIN32(GetLastError() ? GetLastError() : ERROR_NOT_FOUND);
        } else if (m->wParam==12) {
            source=GetPropW(m->hwnd,L"Ntw16s.XamlHost.v1") ? 1 : 0; hr=S_OK;
        } else if (m->wParam==13 && fixture_window) {
            window_native *native=NULL; HWND fixture=NULL;
            hr=fixture_window->vtable->query(fixture_window,&window_native_iid,(void **)&native);
            if (SUCCEEDED(hr)) hr=native->vtable->get_hwnd(native,&fixture);
            if (SUCCEEDED(hr) && fixture) {
                HMODULE product=GetModuleHandleW(sizeof(void *)==8 ? L"ntw16s64.dll" : L"ntw16s32.dll");
                BOOL (__cdecl *attached)(HWND)=(void *)(uintptr_t)(product ? GetProcAddress(product,"Ntw16sWindowAttached") : NULL);
                if (!attached) hr=E_FAIL;
                else {
                    SetPropW(m->hwnd,L"Win16Xaml.Test.FixtureHwnd",fixture);
                    SetPropW(m->hwnd,L"Win16Xaml.Test.FixtureHost",GetPropW(fixture,L"Ntw16s.XamlHost.v1"));
                    source=attached(fixture) ? 1 : 0;
                }
            } else if (SUCCEEDED(hr)) hr=E_FAIL;
            if (native) native->vtable->release(native);
        } else if (m->wParam==28 && button) {
            input_element *element=NULL; item_mapping *mapping=NULL;
            hr=IInspectable_QueryInterface(button,&ui_iid,(void **)&element);
            if (SUCCEEDED(hr)) hr=element->vtable->update_layout(element);
            if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&item_mapping_iid,(void **)&mapping);
            if (SUCCEEDED(hr)) hr=mapping->vtable->container_from_index(mapping,0,&other_button);
            if (SUCCEEDED(hr) && !other_button) hr=E_FAIL;
            if (mapping) mapping->vtable->release(mapping);
            if (element) element->vtable->release(element);
        } else if (m->wParam==22 && button) {
            input_element *element=NULL; combo_control *combo=NULL; item_mapping *mapping=NULL;
            hr=IInspectable_QueryInterface(button,&ui_iid,(void **)&element);
            if (SUCCEEDED(hr)) hr=element->vtable->update_layout(element);
            if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&combo_control_iid,(void **)&combo);
            if (SUCCEEDED(hr)) hr=combo->vtable->put_open(combo,1);
            if (SUCCEEDED(hr)) hr=element->vtable->update_layout(element);
            if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&item_mapping_iid,(void **)&mapping);
            if (SUCCEEDED(hr)) hr=mapping->vtable->container_from_index(mapping,0,&other_button);
            if (SUCCEEDED(hr) && !other_button) hr=E_FAIL;
            if (mapping) mapping->vtable->release(mapping);
            if (combo) combo->vtable->release(combo);
            if (element) element->vtable->release(element);
        } else if (m->wParam==23 && other_button && service && diagnostics) {
            hr=win16_xaml_button_source(service,diagnostics,other_button,&source,&index);
        } else if (m->wParam==19 && button) {
            input_element *input=NULL; boolean focused=0;
            hr=IInspectable_QueryInterface(button,&ui_iid,(void **)&input);
            if (SUCCEEDED(hr)) hr=input->vtable->focus(input,1,&focused);
            if (SUCCEEDED(hr)) source=focused ? 1 : 0;
            if (input) input->vtable->release(input);
        } else if (m->wParam==20 && button) {
            input_textbox *input=NULL; HSTRING text=NULL;
            hr=IInspectable_QueryInterface(button,&input_textbox_iid,(void **)&input);
            if (SUCCEEDED(hr)) hr=input->vtable->get_text(input,&text);
            if (SUCCEEDED(hr)) source=text && !wcscmp(WindowsGetStringRawBuffer(text,NULL),L"z") ? 1 : 0;
            if (text) WindowsDeleteString(text);
            if (input) input->vtable->release(input);
        } else if (m->wParam==14) {
            win16_xaml_adapter *adapter=(win16_xaml_adapter *)m->lParam;
            if (!other_button && adapter) {
                for (observed_entry *entry=(observed_entry *)adapter->session.buttons.first;entry;entry=(observed_entry *)entry->next) {
                    HSTRING runtime=NULL;
                    if (entry->style.button && SUCCEEDED(IInspectable_GetRuntimeClassName(entry->style.button,&runtime)) && runtime &&
                        !wcscmp(WindowsGetStringRawBuffer(runtime,NULL),L"Microsoft.UI.Xaml.Controls.Button")) {
                        other_button=entry->style.button; IInspectable_AddRef(other_button);
                    }
                    if (runtime) WindowsDeleteString(runtime);
                    if (other_button) break;
                }
            }
            if (other_button && service && diagnostics)
                hr=win16_xaml_button_source(service,diagnostics,other_button,&source,&index);
        } else if (m->wParam==15) {
            HWND owner=NULL; hr=win16_xaml_element_owner(button,&owner);
            if (SUCCEEDED(hr)) { SetPropW(m->hwnd,L"Win16Xaml.Test.OwnerHwnd",owner); source=1; }
        } else if (m->wParam==16) {
            HWND owner=NULL; hr=win16_xaml_element_owner(other_button,&owner);
            if (SUCCEEDED(hr)) { SetPropW(m->hwnd,L"Win16Xaml.Test.OwnerHwnd",owner); source=1; }
        } else if (m->wParam==2 && button) {
            hr=win16_xaml_button_source(service,diagnostics,button,&source,&index);
            clear();
        }
        uintptr_t value=SUCCEEDED(hr) ? (uintptr_t)source+1 : ((uintptr_t)(DWORD)hr<<1)|1;
        if (m->wParam==5) {
            SetPropW(m->hwnd,L"Win16Xaml.Test.Property.Phase",(HANDLE)(uintptr_t)fixture_phase);
            SetPropW(m->hwnd,L"Win16Xaml.Test.Property.Nodes",(HANDLE)(uintptr_t)fixture_nodes);
            SetPropW(m->hwnd,L"Win16Xaml.Test.Property.Depth",(HANDLE)(uintptr_t)fixture_max_depth);
            SetPropW(m->hwnd,L"Win16Xaml.Test.Property.Entries",(HANDLE)(uintptr_t)fixture_entries);
        }
        SetPropW(m->hwnd,L"Win16Xaml.Test.Property.Result",(HANDLE)value);
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
