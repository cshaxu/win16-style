/* Test-only XAML Diagnostics TAP. Loaded only by the self-owned fixture. */
#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <ocidl.h>
#include <oleauto.h>
#include <winstring.h>
#include <roapi.h>
#include <xamlom.h>
#include <windows.foundation.h>
#include <wchar.h>
#include <stdio.h>
#include "lib-win32/win16-control-xaml/button_template.h"
#include "lib-win32/win16-control-xaml/checkbox_template.h"
#include "lib-win32/win16-control-xaml/button_style.h"
#include "lib-win32/win16-control-xaml/visual_tree.h"
#include "lib-win32/win16-control-xaml/session.h"
#include "lib-win32/win16-control-xaml/diagnostics_site.h"
#include "lib-win32/win16-control-xaml/diagnostics_subscribe.h"
#include "lib-win32/win16-control-xaml/diagnostics_factory.h"
#include "lib-win32/win16-control-xaml/adapter.h"

static const CLSID tap_clsid = {0x7132a744,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};
static IVisualTreeService *service;
static IXamlDiagnostics *diagnostics;
static IInspectable *found_button;
static LONG button_seen;
static LONG site_seen, changes;
static LONG style_attempted;
static UINT_PTR style_timer;
static InstanceHandle style_target;
static unsigned int style_index;
static IInspectable *style_button;
static IInspectable *style_alternate;
static IInspectable *style_expected;
static win16_xaml_button_set style_set;
static win16_xaml_button_style *style_state;
static win16_xaml_button_style *second_style_state;
static LONG second_in_progress;
static IUnknown *callback_marshaler;
static win16_xaml_session reusable_session;
static UINT_PTR reusable_timer;
static BOOL reusable_finished;
static IInspectable *reusable_button;
static IObjectWithSite *generic_site;
static win16_xaml_adapter reusable_adapter;
static HANDLE site_thread_tag;
static HANDLE callback_thread_tag;
static LONG template_parse_attempted;

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
typedef struct winui_element winui_element;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(winui_element *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(winui_element *);
    ULONG (STDMETHODCALLTYPE *release)(winui_element *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(winui_element *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(winui_element *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(winui_element *, TrustLevel *);
    void (STDMETHODCALLTYPE *unused[46])(void);
    HRESULT (STDMETHODCALLTYPE *get_style)(winui_element *, IInspectable **);
    HRESULT (STDMETHODCALLTYPE *put_style)(winui_element *, IInspectable *);
} winui_element_vtable;
struct winui_element { const winui_element_vtable *vtable; };
static const IID winui_reader_iid = {0x82a4cd9e,0x435e,0x5aeb,{0x8c,0x4f,0x30,0x0c,0xec,0xe4,0x5c,0xae}};
static const IID winui_element_iid = {0xfe08f13d,0xdc6a,0x5495,{0xad,0x44,0xc2,0xd8,0xd2,0x18,0x63,0xb0}};
static const IID winui_style_iid = {0x65e1d164,0x572f,0x5b0e,{0xa8,0x0f,0x9c,0x02,0x44,0x1f,0xac,0x49}};
static const IID winui_dependency_iid = {0xe7beaee7,0x160e,0x50f7,{0x87,0x89,0xd6,0x34,0x63,0xf9,0x79,0xfa}};
static const IID winui_tree_iid = {0x5aece43c,0x7651,0x5bb5,{0x85,0x5c,0x21,0x98,0x49,0x6e,0x45,0x5e}};
static const IID winui_control_iid = {0x857d6e8a,0xd45a,0x5c69,{0xa9,0x9c,0xbf,0x6a,0x5c,0x54,0xfb,0x38}};
static const IID winui_vsm_iid = {0xa4d5147d,0x88c3,0x57ed,{0xad,0x83,0x24,0x5d,0xf5,0xf6,0xb5,0x0d}};
static const IID winui_content_iid = {0x07e81761,0x11b2,0x52ae,{0x8f,0x8b,0x4d,0x53,0xd2,0xb5,0x90,0x0a}};
static const IID winui_presenter_iid = {0x075e6ca8,0xda46,0x5ec6,{0x8c,0x64,0x03,0xb3,0xdd,0xea,0xf7,0x4f}};
static const IID property_value_iid = {0x4bd682dd,0x7554,0x40e9,{0x9a,0x9b,0x82,0x65,0x4e,0xde,0x7e,0x62}};
HRESULT WINAPI TestStyleSource(IInspectable *,unsigned int *,unsigned int *,unsigned int *);

typedef struct winui_tree winui_tree;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(winui_tree *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(winui_tree *);
    ULONG (STDMETHODCALLTYPE *release)(winui_tree *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(winui_tree *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(winui_tree *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(winui_tree *, TrustLevel *);
    void (STDMETHODCALLTYPE *unused[4])(void);
    HRESULT (STDMETHODCALLTYPE *get_child)(winui_tree *, IInspectable *, INT32, IInspectable **);
    HRESULT (STDMETHODCALLTYPE *get_count)(winui_tree *, IInspectable *, INT32 *);
} winui_tree_vtable;
struct winui_tree { const winui_tree_vtable *vtable; };

typedef struct content_interface content_interface;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(content_interface *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(content_interface *);
    ULONG (STDMETHODCALLTYPE *release)(content_interface *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(content_interface *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(content_interface *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(content_interface *, TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_content)(content_interface *, IInspectable **);
} content_vtable;
struct content_interface { const content_vtable *vtable; };

static int check_button_content(IInspectable *button)
{
    static const wchar_t type_name[] = L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    HSTRING name=NULL;
    winui_tree *tree=NULL;
    IInspectable *dependency=NULL,*border=NULL,*border_dependency=NULL,*grid=NULL,*grid_dependency=NULL,*presenter=NULL,*a=NULL,*b=NULL;
    IUnknown *a_identity=NULL,*b_identity=NULL;
    content_interface *control=NULL,*view=NULL;
    HRESULT hr=WindowsCreateString(type_name,(UINT32)(ARRAYSIZE(type_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&winui_dependency_iid,(void **)&dependency);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dependency,0,&border);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(border,&winui_dependency_iid,(void **)&border_dependency);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,border_dependency,0,&grid);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(grid,&winui_dependency_iid,(void **)&grid_dependency);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,grid_dependency,5,&presenter);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&winui_content_iid,(void **)&control);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(presenter,&winui_presenter_iid,(void **)&view);
    if (SUCCEEDED(hr)) hr=control->vtable->get_content(control,&a);
    if (SUCCEEDED(hr)) hr=view->vtable->get_content(view,&b);
    if (SUCCEEDED(hr) && a && b) hr=IInspectable_QueryInterface(a,&IID_IUnknown,(void **)&a_identity);
    if (SUCCEEDED(hr) && a && b) hr=IInspectable_QueryInterface(b,&IID_IUnknown,(void **)&b_identity);
    int result=FAILED(hr) ? 5 : !a ? 2 : !b ? 3 : a_identity==b_identity ? 1 : 4;
    if (result==4) {
        __x_ABI_CWindows_CFoundation_CIPropertyValue *av=NULL,*bv=NULL;
        HSTRING a_text=NULL,b_text=NULL;
        HRESULT text_hr=IInspectable_QueryInterface(a,&property_value_iid,(void **)&av);
        if (SUCCEEDED(text_hr)) text_hr=IInspectable_QueryInterface(b,&property_value_iid,(void **)&bv);
        if (SUCCEEDED(text_hr)) text_hr=__x_ABI_CWindows_CFoundation_CIPropertyValue_GetString(av,&a_text);
        if (SUCCEEDED(text_hr)) text_hr=__x_ABI_CWindows_CFoundation_CIPropertyValue_GetString(bv,&b_text);
        if (SUCCEEDED(text_hr) && !wcscmp(WindowsGetStringRawBuffer(a_text,NULL),WindowsGetStringRawBuffer(b_text,NULL))) result=6;
        if (a_text) WindowsDeleteString(a_text);
        if (b_text) WindowsDeleteString(b_text);
        if (av) __x_ABI_CWindows_CFoundation_CIPropertyValue_Release(av);
        if (bv) __x_ABI_CWindows_CFoundation_CIPropertyValue_Release(bv);
    }
    if (a_identity) IUnknown_Release(a_identity);
    if (b_identity) IUnknown_Release(b_identity);
    if (a) IInspectable_Release(a);
    if (b) IInspectable_Release(b);
    if (view) view->vtable->release(view);
    if (control) control->vtable->release(control);
    if (presenter) IInspectable_Release(presenter);
    if (grid_dependency) IInspectable_Release(grid_dependency);
    if (grid) IInspectable_Release(grid);
    if (border_dependency) IInspectable_Release(border_dependency);
    if (border) IInspectable_Release(border);
    if (dependency) IInspectable_Release(dependency);
    if (tree) tree->vtable->release(tree);
    if (name) WindowsDeleteString(name);
    return result;
}

typedef struct winui_vsm winui_vsm;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(winui_vsm *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(winui_vsm *);
    ULONG (STDMETHODCALLTYPE *release)(winui_vsm *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(winui_vsm *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(winui_vsm *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(winui_vsm *, TrustLevel *);
    void (STDMETHODCALLTYPE *unused[4])(void);
    HRESULT (STDMETHODCALLTYPE *go_to_state)(winui_vsm *, IInspectable *, HSTRING, boolean, boolean *);
} winui_vsm_vtable;
struct winui_vsm { const winui_vsm_vtable *vtable; };

typedef struct test_border test_border;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_border *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_border *);
    ULONG (STDMETHODCALLTYPE *release)(test_border *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_border *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_border *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_border *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[4])(void);
    HRESULT (STDMETHODCALLTYPE *get_background)(test_border *,IInspectable **);
} test_border_vtable;
struct test_border { const test_border_vtable *vtable; };
typedef struct test_brush test_brush;
typedef struct { BYTE a,r,g,b; } test_color;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_brush *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_brush *);
    ULONG (STDMETHODCALLTYPE *release)(test_brush *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_brush *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_brush *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_brush *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_color)(test_brush *,test_color *);
} test_brush_vtable;
struct test_brush { const test_brush_vtable *vtable; };
static const IID border_iid={0x1ca13b47,0xff5c,0x5abc,{0xa4,0x11,0xa1,0x77,0xdf,0x94,0x82,0xa9}};
static const IID brush_iid={0xb3865c31,0x37c8,0x55c1,{0x8a,0x72,0xd4,0x1c,0x67,0x64,0x2e,0x2a}};

typedef struct test_presenter test_presenter;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_presenter *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_presenter *);
    ULONG (STDMETHODCALLTYPE *release)(test_presenter *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_presenter *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_presenter *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_presenter *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[20])(void);
    HRESULT (STDMETHODCALLTYPE *get_foreground)(test_presenter *,IInspectable **);
} test_presenter_vtable;
struct test_presenter { const test_presenter_vtable *vtable; };

static BOOL check_disabled_text(IInspectable *button)
{
    static const wchar_t type_name[]=L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    HSTRING name=NULL; winui_tree *tree=NULL;
    IInspectable *dependency=NULL,*frame=NULL,*frame_dep=NULL,*grid=NULL,*grid_dep=NULL,*object=NULL,*foreground=NULL;
    test_presenter *presenter=NULL; test_brush *brush=NULL; test_color color={0};
    HRESULT hr=WindowsCreateString(type_name,(UINT32)(ARRAYSIZE(type_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&winui_dependency_iid,(void **)&dependency);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dependency,0,&frame);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(frame,&winui_dependency_iid,(void **)&frame_dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,frame_dep,0,&grid);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(grid,&winui_dependency_iid,(void **)&grid_dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,grid_dep,5,&object);
    if (SUCCEEDED(hr) && !object) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_presenter_iid,(void **)&presenter);
    if (SUCCEEDED(hr)) hr=presenter->vtable->get_foreground(presenter,&foreground);
    if (SUCCEEDED(hr) && !foreground) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(foreground,&brush_iid,(void **)&brush);
    if (SUCCEEDED(hr)) hr=brush->vtable->get_color(brush,&color);
    if (SUCCEEDED(hr) && (color.a!=255 || color.r!=128 || color.g!=128 || color.b!=128)) hr=E_FAIL;
    if (brush) brush->vtable->release(brush);
    if (foreground) IInspectable_Release(foreground);
    if (presenter) presenter->vtable->release(presenter);
    if (object) IInspectable_Release(object);
    if (grid_dep) IInspectable_Release(grid_dep);
    if (grid) IInspectable_Release(grid);
    if (frame_dep) IInspectable_Release(frame_dep);
    if (frame) IInspectable_Release(frame);
    if (dependency) IInspectable_Release(dependency);
    if (tree) tree->vtable->release(tree);
    if (name) WindowsDeleteString(name);
    return SUCCEEDED(hr);
}

static BOOL check_edge_colors(IInspectable *button,BOOL pressed)
{
    static const wchar_t type_name[]=L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    HSTRING name=NULL; winui_tree *tree=NULL;
    IInspectable *dependency=NULL,*frame=NULL,*frame_dep=NULL,*grid=NULL,*grid_dep=NULL;
    HRESULT hr=WindowsCreateString(type_name,(UINT32)(ARRAYSIZE(type_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&winui_dependency_iid,(void **)&dependency);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dependency,0,&frame);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(frame,&winui_dependency_iid,(void **)&frame_dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,frame_dep,0,&grid);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(grid,&winui_dependency_iid,(void **)&grid_dep);
    for (int i=0;SUCCEEDED(hr) && i<4;i++) {
        IInspectable *edge=NULL,*background=NULL; test_border *border=NULL; test_brush *brush=NULL;
        test_color color={0};
        hr=tree->vtable->get_child(tree,grid_dep,i,&edge);
        if (SUCCEEDED(hr) && !edge) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(edge,&border_iid,(void **)&border);
        if (SUCCEEDED(hr)) hr=border->vtable->get_background(border,&background);
        if (SUCCEEDED(hr) && !background) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(background,&brush_iid,(void **)&brush);
        if (SUCCEEDED(hr)) hr=brush->vtable->get_color(brush,&color);
        BYTE expected=(BYTE)(((i<2) != pressed) ? 255 : 128);
        if (SUCCEEDED(hr) && (color.a!=255 || color.r!=expected || color.g!=expected || color.b!=expected)) hr=E_FAIL;
        if (brush) brush->vtable->release(brush);
        if (background) IInspectable_Release(background);
        if (border) border->vtable->release(border);
        if (edge) IInspectable_Release(edge);
    }
    if (grid_dep) IInspectable_Release(grid_dep);
    if (grid) IInspectable_Release(grid);
    if (frame_dep) IInspectable_Release(frame_dep);
    if (frame) IInspectable_Release(frame);
    if (dependency) IInspectable_Release(dependency);
    if (tree) tree->vtable->release(tree);
    if (name) WindowsDeleteString(name);
    return SUCCEEDED(hr);
}

static BOOL check_button_states(IInspectable *button)
{
    static const wchar_t type_name[] = L"Microsoft.UI.Xaml.VisualStateManager";
    static const wchar_t *states[] = {L"Normal",L"PointerOver",L"Pressed",L"Disabled",L"Normal",L"Focused",L"Unfocused"};
    HSTRING name=NULL;
    winui_vsm *manager=NULL;
    IInspectable *control=NULL;
    HRESULT hr=WindowsCreateString(type_name,(UINT32)(ARRAYSIZE(type_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&winui_control_iid,(void **)&control);
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        HSTRING state=NULL;
        boolean changed=0;
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr) && (i==0 || i==2) && !check_edge_colors(button,i==2)) hr=E_FAIL;
        if (SUCCEEDED(hr) && i==3 && !check_disabled_text(button)) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    if (control) IInspectable_Release(control);
    if (manager) manager->vtable->release(manager);
    if (name) WindowsDeleteString(name);
    return SUCCEEDED(hr);
}

static BOOL has_test_border(IInspectable *button)
{
    static const wchar_t type_name[] = L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    HSTRING name=NULL, child_name=NULL;
    winui_tree *tree=NULL;
    IInspectable *dependency=NULL,*child=NULL;
    INT32 count=0;
    HRESULT hr=WindowsCreateString(type_name,(UINT32)(ARRAYSIZE(type_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(button,&winui_dependency_iid,(void **)&dependency);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_count(tree,dependency,&count);
    if (SUCCEEDED(hr) && count==1) hr=tree->vtable->get_child(tree,dependency,0,&child);
    if (SUCCEEDED(hr) && child) hr=IInspectable_GetRuntimeClassName(child,&child_name);
    BOOL found=SUCCEEDED(hr) && child_name && !wcscmp(WindowsGetStringRawBuffer(child_name,NULL),L"Microsoft.UI.Xaml.Controls.Border");
    if (child_name) WindowsDeleteString(child_name);
    if (child) IInspectable_Release(child);
    if (dependency) IInspectable_Release(dependency);
    if (tree) tree->vtable->release(tree);
    if (name) WindowsDeleteString(name);
    return found;
}

static int check_rendered_pixels(IInspectable *button,HRESULT *result)
{
    IVisualTreeService2 *service2=NULL;
    IBitmapData *bitmap=NULL;
    InstanceHandle handle=0;
    BitmapDescription description={0};
    unsigned int stride=0, copied=0;
    byte *pixels=NULL;
    int stage=2;
    HRESULT hr=IVisualTreeService_QueryInterface(service,&IID_IVisualTreeService2,(void **)&service2);
    if (SUCCEEDED(hr)) { stage=3; hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,button,&handle); }
    if (SUCCEEDED(hr)) { stage=4; hr=IVisualTreeService2_RenderTargetBitmap(service2,handle,RenderTargetAndChildren,512,512,&bitmap); }
    if (SUCCEEDED(hr)) { stage=5; hr=IBitmapData_GetBitmapDescription(bitmap,&description); }
    if (SUCCEEDED(hr)) { stage=6; hr=IBitmapData_GetStride(bitmap,&stride); }
    if (SUCCEEDED(hr)) stage=7;
    if (SUCCEEDED(hr) && (!description.Width || !description.Height || !stride ||
        stride>1024*1024 || description.Height>(1024*1024)/stride)) hr=E_FAIL;
    if (SUCCEEDED(hr)) {
        stage=8;
        unsigned int size=stride*description.Height;
        pixels=HeapAlloc(GetProcessHeap(),0,size);
        if (!pixels) hr=E_OUTOFMEMORY;
        else hr=IBitmapData_CopyBytesTo(bitmap,0,size,pixels,&copied);
        if (SUCCEEDED(hr) && copied!=size) hr=E_FAIL;
    }
    if (pixels) HeapFree(GetProcessHeap(),0,pixels);
    if (bitmap) IBitmapData_Release(bitmap);
    if (service2) IVisualTreeService2_Release(service2);
    *result=hr;
    return SUCCEEDED(hr) ? 1 : stage;
}

static BOOL button_class(const wchar_t *name)
{
    return name && (!wcscmp(name,L"Windows.UI.Xaml.Controls.Button") ||
                    !wcscmp(name,L"Microsoft.UI.Xaml.Controls.Button"));
}

static void signal_named(const wchar_t *variable)
{
    WCHAR name[160];
    DWORD length = GetEnvironmentVariableW(variable,name,ARRAYSIZE(name));
    if (!length || length >= ARRAYSIZE(name)) return;
    HANDLE event = OpenEventW(EVENT_MODIFY_STATE,FALSE,name);
    if (event) { SetEvent(event); CloseHandle(event); }
}

static void CALLBACK restore_style_timer(HWND window, UINT message, UINT_PTR timer, DWORD time)
{
    (void)window; (void)message; (void)time;
    KillTimer(NULL,timer); style_timer=0;
    if (second_style_state) {
        IInspectable *second_button=second_style_state->button;
        IInspectable_AddRef(second_button);
        HRESULT second=win16_xaml_button_set_remove(&style_set,service,diagnostics,second_style_state);
        if (SUCCEEDED(second)) {
            unsigned int matches=0,source=0,index=0;
            second_style_state=NULL;
            second=TestStyleSource(second_button,&matches,&source,&index);
            if (SUCCEEDED(second) && matches==1 && source==BaseValueSourceDefault)
                signal_named(L"WIN16_XAML_SECOND_RESTORED_EVENT");
            else signal_named(L"WIN16_XAML_FAILED_EVENT");
        }
        else signal_named(L"WIN16_XAML_FAILED_EVENT");
        IInspectable_Release(second_button);
    }
    winui_element *element=NULL;
    IInspectable *current=NULL;
    IUnknown *a=NULL,*b=NULL;
    HRESULT hr = style_button ? IInspectable_QueryInterface(style_button,&winui_element_iid,(void **)&element) : E_FAIL;
    if (FAILED(hr)) signal_named(L"WIN16_XAML_RESTORE_QUERY_FAIL_EVENT");
    if (SUCCEEDED(hr)) hr=element->vtable->get_style(element,&current);
    if (FAILED(hr)) signal_named(L"WIN16_XAML_RESTORE_GET_FAIL_EVENT");
    if (SUCCEEDED(hr) && !current) { hr=E_FAIL; signal_named(L"WIN16_XAML_RESTORE_CURRENT_FAIL_EVENT"); }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(current,&IID_IUnknown,(void **)&a);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(style_expected,&IID_IUnknown,(void **)&b);
    if (FAILED(hr)) signal_named(L"WIN16_XAML_RESTORE_EXPECTED_FAIL_EVENT");
    if (SUCCEEDED(hr) && a==b && has_test_border(style_button)) {
        signal_named(L"WIN16_XAML_TEMPLATE_TREE_EVENT");
        HRESULT bitmap_result=S_OK;
        int bitmap=check_rendered_pixels(style_button,&bitmap_result);
        if (bitmap==1) signal_named(L"WIN16_XAML_BITMAP_OK_EVENT");
        else {
            WCHAR variable[48];
            signal_named(L"WIN16_XAML_BITMAP_FAILED_EVENT");
            if (bitmap_result==E_NOTIMPL) signal_named(L"WIN16_XAML_BITMAP_NOTIMPL_EVENT");
            else if (bitmap_result==E_ACCESSDENIED) signal_named(L"WIN16_XAML_BITMAP_DENIED_EVENT");
            else if (bitmap_result==E_INVALIDARG) signal_named(L"WIN16_XAML_BITMAP_INVALID_EVENT");
            else if (bitmap_result==E_FAIL) signal_named(L"WIN16_XAML_BITMAP_EFAIL_EVENT");
            else signal_named(L"WIN16_XAML_BITMAP_OTHER_EVENT");
            swprintf(variable,ARRAYSIZE(variable),L"WIN16_XAML_BITMAP_STAGE_%d_EVENT",bitmap);
            signal_named(variable);
        }
        int content=check_button_content(style_button);
        if (content==1) signal_named(L"WIN16_XAML_CONTENT_MATCH_EVENT");
        else if (content==2) signal_named(L"WIN16_XAML_CONTENT_SOURCE_EMPTY_EVENT");
        else if (content==3) signal_named(L"WIN16_XAML_CONTENT_VIEW_EMPTY_EVENT");
        else if (content==4) signal_named(L"WIN16_XAML_CONTENT_DIFFERENT_EVENT");
        else if (content==6) signal_named(L"WIN16_XAML_CONTENT_TEXT_MATCH_EVENT");
        else signal_named(L"WIN16_XAML_CONTENT_CHECK_FAILED_EVENT");
        if (check_button_states(style_button)) signal_named(L"WIN16_XAML_STATES_EVENT");
    }
    if (SUCCEEDED(hr) && a!=b) {
        IUnknown *alternate_identity=NULL;
        HRESULT conflict=win16_xaml_button_set_remove(&style_set,service,diagnostics,style_state);
        if (conflict==HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH)) signal_named(L"WIN16_XAML_CONFLICT_EVENT");
        else signal_named(L"WIN16_XAML_FAILED_EVENT");
        if (style_alternate && SUCCEEDED(IInspectable_QueryInterface(style_alternate,&IID_IUnknown,(void **)&alternate_identity))) {
            if (a==alternate_identity) signal_named(L"WIN16_XAML_PRESERVED_EVENT");
            IUnknown_Release(alternate_identity);
        }
        hr=E_ABORT;
    }
    if (SUCCEEDED(hr)) {
        hr=win16_xaml_button_set_remove(&style_set,service,diagnostics,style_state);
        if (SUCCEEDED(hr)) style_state=NULL;
        if (FAILED(hr)) signal_named(L"WIN16_XAML_CLEAR_FAILED_EVENT");
    }
    unsigned int matches=0, source=0, index=0;
    if (SUCCEEDED(hr)) {
        hr=TestStyleSource(style_button,&matches,&source,&index);
        if (FAILED(hr)) signal_named(L"WIN16_XAML_SOURCE_FAILED_EVENT");
        else if (matches!=1 || source!=BaseValueSourceDefault) signal_named(L"WIN16_XAML_SOURCE_MISMATCH_EVENT");
    }
    if (SUCCEEDED(hr) && matches==1 && source==BaseValueSourceDefault) signal_named(L"WIN16_XAML_RESTORED_EVENT");
    else if (hr!=E_ABORT) {
        if (style_alternate) {
            winui_element *element=NULL;
            IInspectable *actual=NULL;
            IUnknown *actual_identity=NULL,*alternate_identity=NULL;
            HRESULT check=IInspectable_QueryInterface(style_button,&winui_element_iid,(void **)&element);
            if (SUCCEEDED(check)) check=element->vtable->get_style(element,&actual);
            if (SUCCEEDED(check) && actual) check=IInspectable_QueryInterface(actual,&IID_IUnknown,(void **)&actual_identity);
            if (SUCCEEDED(check)) check=IInspectable_QueryInterface(style_alternate,&IID_IUnknown,(void **)&alternate_identity);
            if (SUCCEEDED(check) && actual_identity && actual_identity==alternate_identity) signal_named(L"WIN16_XAML_PRESERVED_EVENT");
            if (actual_identity) IUnknown_Release(actual_identity);
            if (alternate_identity) IUnknown_Release(alternate_identity);
            if (actual) IInspectable_Release(actual);
            if (element) element->vtable->release(element);
        }
        signal_named(L"WIN16_XAML_RESTORE_FAILED_EVENT"); signal_named(L"WIN16_XAML_FAILED_EVENT");
    }
    if (a) IUnknown_Release(a);
    if (b) IUnknown_Release(b);
    if (current) IInspectable_Release(current);
    if (element) element->vtable->release(element);
    if (style_button) { IInspectable_Release(style_button); style_button=NULL; }
    if (style_alternate) { IInspectable_Release(style_alternate); style_alternate=NULL; }
    if (style_expected) { IInspectable_Release(style_expected); style_expected=NULL; }
    win16_xaml_button_set_discard(&style_set); style_state=NULL; second_style_state=NULL;
}

static void CALLBACK change_style_timer(HWND window, UINT message, UINT_PTR timer, DWORD time)
{
    (void)window; (void)message; (void)time;
    KillTimer(NULL,timer);
    winui_element *element=NULL;
    IInspectable *replacement=NULL;
    HRESULT hr=IInspectable_QueryInterface(style_button,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(style_alternate,&winui_style_iid,(void **)&replacement);
    if (FAILED(hr)) signal_named(L"WIN16_XAML_CHANGE_HANDLE_FAILED_EVENT");
    if (SUCCEEDED(hr)) hr=element->vtable->put_style(element,replacement);
    if (FAILED(hr)) signal_named(L"WIN16_XAML_CHANGE_SET_FAILED_EVENT");
    signal_named(SUCCEEDED(hr) ? L"WIN16_XAML_CHANGED_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    if (replacement) IInspectable_Release(replacement);
    if (element) element->vtable->release(element);
}

static DWORD WINAPI test_wrong_thread(LPVOID context)
{
    return (DWORD)win16_xaml_button_set_discard((win16_xaml_button_set *)context);
}

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
typedef struct test_ui test_ui;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_ui *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_ui *);
    ULONG (STDMETHODCALLTYPE *release)(test_ui *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_ui *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_ui *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_ui *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_opacity)(test_ui *,double *);
} test_ui_vtable;
struct test_ui { const test_ui_vtable *vtable; };
static const IID test_ui_iid={0xc3c01020,0x320c,0x5cf6,{0x9d,0x24,0xd3,0x96,0xbb,0xfa,0x4d,0x8b}};
typedef struct test_toggle test_toggle;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_toggle *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_toggle *);
    ULONG (STDMETHODCALLTYPE *release)(test_toggle *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_toggle *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_toggle *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_toggle *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_checked)(test_toggle *,IUnknown **);
    HRESULT (STDMETHODCALLTYPE *put_checked)(test_toggle *,IUnknown *);
    HRESULT (STDMETHODCALLTYPE *get_three)(test_toggle *,boolean *);
    HRESULT (STDMETHODCALLTYPE *put_three)(test_toggle *,boolean);
} test_toggle_vtable;
struct test_toggle { const test_toggle_vtable *vtable; };
static const IID test_toggle_iid={0x686fbaa4,0xc866,0x568b,{0x8f,0x75,0x48,0x1d,0x8d,0x54,0x52,0x91}};
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
static const IID test_window_iid={0x61f0ec79,0x5d52,0x56b5,{0x86,0xfb,0x40,0xfa,0x4a,0xf2,0x88,0xb0}};

static HRESULT check_checkbox_states(IInspectable *checkbox)
{
    static const wchar_t tree_name[]=L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    static const wchar_t vsm_name[]=L"Microsoft.UI.Xaml.VisualStateManager";
    static const wchar_t *states[]={L"Unchecked",L"Checked",L"Indeterminate"};
    static const double expected[][2]={{0,0},{1,0},{0,1}};
    HSTRING name=NULL; winui_tree *tree=NULL; winui_vsm *manager=NULL; test_control *control=NULL;
    test_toggle *toggle=NULL; __x_ABI_CWindows_CFoundation_CIPropertyValueStatics *properties=NULL;
    IInspectable *root=NULL,*box=NULL,*grid=NULL,*mark=NULL,*mixed=NULL,*focus=NULL,*dep=NULL;
    test_ui *ui[2]={0},*focus_ui=NULL; boolean applied=0;
    HRESULT hr=IInspectable_QueryInterface(checkbox,&winui_control_iid,(void **)&control);
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(tree_name,(UINT32)(ARRAYSIZE(tree_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(checkbox,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (dep) { IInspectable_Release(dep); dep=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&box);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,1,&focus);
    if (dep) { IInspectable_Release(dep); dep=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(focus,&test_ui_iid,(void **)&focus_ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(box,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&grid);
    if (dep) { IInspectable_Release(dep); dep=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(grid,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&mark);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,1,&mixed);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(mark,&test_ui_iid,(void **)&ui[0]);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(mixed,&test_ui_iid,(void **)&ui[1]);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(vsm_name,(UINT32)(ARRAYSIZE(vsm_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    static const wchar_t *common_states[]={L"Normal",L"PointerOver",L"Pressed",L"Disabled"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(common_states);i++) {
        HSTRING state=NULL; boolean changed=0;
        hr=WindowsCreateString(common_states[i],(UINT32)lstrlenW(common_states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    if (SUCCEEDED(hr)) {
        HSTRING state=NULL; boolean changed=0;
        hr=WindowsCreateString(L"Normal",6,&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    static const wchar_t *focus_states[]={L"Focused",L"Unfocused"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(focus_states);i++) {
        HSTRING state=NULL; boolean changed=0; double opacity=-1;
        hr=WindowsCreateString(focus_states[i],(UINT32)lstrlenW(focus_states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=focus_ui->vtable->get_opacity(focus_ui,&opacity);
        if (SUCCEEDED(hr) && opacity!=(i==0 ? 1.0 : 0.0)) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        HSTRING state=NULL; boolean changed=0;
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        for (unsigned int j=0;SUCCEEDED(hr) && j<2;j++) {
            double opacity=-1; hr=ui[j]->vtable->get_opacity(ui[j],&opacity);
            if (SUCCEEDED(hr) && opacity!=expected[i][j]) hr=E_FAIL;
        }
        if (state) WindowsDeleteString(state);
    }
    if (SUCCEEDED(hr)) {
        HSTRING state=NULL; boolean changed=0;
        hr=WindowsCreateString(L"Unchecked",9,&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Windows.Foundation.PropertyValue",32,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics,(void **)&properties);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(checkbox,&test_toggle_iid,(void **)&toggle);
    if (SUCCEEDED(hr)) hr=toggle->vtable->put_three(toggle,1);
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        IInspectable *boxed=NULL; IUnknown *reference=NULL;
        if (i<2) hr=__x_ABI_CWindows_CFoundation_CIPropertyValueStatics_CreateBoolean(properties,(boolean)i,&boxed);
        if (SUCCEEDED(hr) && boxed) hr=IInspectable_QueryInterface(boxed,&IID___FIReference_1_boolean,(void **)&reference);
        if (SUCCEEDED(hr)) hr=toggle->vtable->put_checked(toggle,reference);
        for (unsigned int j=0;SUCCEEDED(hr) && j<2;j++) {
            double opacity=-1; hr=ui[j]->vtable->get_opacity(ui[j],&opacity);
            if (SUCCEEDED(hr) && opacity!=expected[i][j]) hr=E_FAIL;
        }
        if (reference) IUnknown_Release(reference);
        if (boxed) IInspectable_Release(boxed);
    }
    if (toggle) toggle->vtable->release(toggle);
    if (properties) __x_ABI_CWindows_CFoundation_CIPropertyValueStatics_Release(properties);
    if (ui[1]) ui[1]->vtable->release(ui[1]);
    if (ui[0]) ui[0]->vtable->release(ui[0]);
    if (focus_ui) focus_ui->vtable->release(focus_ui);
    if (manager) manager->vtable->release(manager);
    if (dep) IInspectable_Release(dep);
    if (mixed) IInspectable_Release(mixed);
    if (mark) IInspectable_Release(mark);
    if (grid) IInspectable_Release(grid);
    if (box) IInspectable_Release(box);
    if (focus) IInspectable_Release(focus);
    if (root) IInspectable_Release(root);
    if (tree) tree->vtable->release(tree);
    if (control) control->vtable->release(control);
    if (name) WindowsDeleteString(name);
    return hr;
}

static HRESULT test_checkbox_apply(void)
{
    const wchar_t runtime[]=L"Microsoft.UI.Xaml.Controls.CheckBox";
    const wchar_t window_runtime[]=L"Microsoft.UI.Xaml.Window";
    HSTRING name=NULL; IInspectable *checkbox=NULL,*window_object=NULL,*element=NULL;
    test_window *window=NULL; win16_xaml_adapter adapter={0};
    VisualElement element_info={0}; ParentChildRelation relation={0};
    unsigned int source=0,index=0;
    HRESULT hr=WindowsCreateString(runtime,(UINT32)(ARRAYSIZE(runtime)-1),&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&checkbox);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(window_runtime,(UINT32)(ARRAYSIZE(window_runtime)-1),&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(checkbox,&test_ui_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,element);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        element_info.Type=(BSTR)runtime;
        /* Exercise root scanning with a real CheckBox as the root. */
        relation.Parent=0;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,checkbox,&element_info.Handle);
        if (SUCCEEDED(hr)) hr=win16_xaml_adapter_change(&adapter,relation,element_info,Add,NULL);
        if (hr==S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,checkbox,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=check_checkbox_states(checkbox);
    if (SUCCEEDED(hr)) {
        hr=win16_xaml_adapter_change(&adapter,relation,element_info,Remove,NULL);
        if (hr==S_FALSE) hr=E_FAIL;
        if (SUCCEEDED(hr) && adapter.session.thread_id) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,checkbox,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (SUCCEEDED(hr)) {
        relation.Parent=1; /* Also exercise the direct element-notification path. */
        hr=win16_xaml_adapter_change(&adapter,relation,element_info,Add,NULL);
        if (hr==S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,checkbox,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (adapter.service) {
        HRESULT removed=win16_xaml_adapter_stop(&adapter);
        if (SUCCEEDED(hr)) hr=removed;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,checkbox,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) IInspectable_Release(element);
    if (window_object) IInspectable_Release(window_object);
    if (checkbox) IInspectable_Release(checkbox);
    if (name) WindowsDeleteString(name);
    return hr;
}

static void test_template_parse(void)
{
    static const wchar_t reader_name[]=L"Microsoft.UI.Xaml.Markup.XamlReader";
    if (!GetEnvironmentVariableW(L"WIN16_XAML_PARSE_TEST",NULL,0) ||
        InterlockedCompareExchange(&template_parse_attempted,1,0)!=0) return;
    HSTRING name=NULL,text=NULL;
    reader *factory=NULL;
    IInspectable *style=NULL;
    const wchar_t *markup=GetEnvironmentVariableW(L"WIN16_XAML_CHECKBOX_TEST",NULL,0)
        ? win16_xaml_checkbox_template() : win16_xaml_button_template();
    HRESULT hr=WindowsCreateString(reader_name,(UINT32)(ARRAYSIZE(reader_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_reader_iid,(void **)&factory);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(markup,(UINT32)lstrlenW(markup),&text);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,text,&style);
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_CHECKBOX_TEST",NULL,0)) {
        HRESULT checkbox=test_checkbox_apply();
        signal_named(SUCCEEDED(checkbox) ? L"WIN16_XAML_CHECKBOX_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    }
    signal_named(SUCCEEDED(hr) && style ? L"WIN16_XAML_PARSE_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    if (style) IInspectable_Release(style);
    if (factory) factory->vtable->release(factory);
    if (text) WindowsDeleteString(text);
    if (name) WindowsDeleteString(name);
}

/* Test-only: touch one default-style WinUI Button in the self-owned target. */
static void try_winui_style(IInspectable *button)
{
    test_template_parse();
    if (!GetEnvironmentVariableW(L"WIN16_XAML_APPLY_TEST",NULL,0)) return;
    if (InterlockedCompareExchange(&style_attempted,0,0) != 0) {
        if (GetEnvironmentVariableW(L"WIN16_XAML_MULTI_TEST",NULL,0) && style_state && !second_style_state &&
            InterlockedCompareExchange(&second_in_progress,1,0)==0) {
            win16_xaml_button_style *candidate=NULL;
            HRESULT result=win16_xaml_button_set_add(&style_set,service,diagnostics,button,&candidate);
            if (result==S_OK && candidate!=style_state) {
                second_style_state=candidate;
                signal_named(L"WIN16_XAML_SECOND_APPLIED_EVENT");
            }
            InterlockedExchange(&second_in_progress,0);
        }
        return;
    }
    static const wchar_t reader_name[] = L"Microsoft.UI.Xaml.Markup.XamlReader";
    const wchar_t *markup=win16_xaml_button_template();
    unsigned int matches=0, source=0, index=0;
    HRESULT hr = TestStyleSource(button,&matches,&source,&index);
    if (SUCCEEDED(hr) && matches == 1 && source <= 14) {
        WCHAR variable[48];
        swprintf(variable,ARRAYSIZE(variable),L"WIN16_XAML_SOURCE_%u_EVENT",source);
        signal_named(variable);
    }
    if (FAILED(hr) || matches != 1 || source != BaseValueSourceDefault) {
        signal_named(L"WIN16_XAML_SKIPPED_EVENT");
        if (FAILED(hr)) signal_named(L"WIN16_XAML_SKIP_QUERY_FAILED_EVENT");
        else if (matches != 1) signal_named(L"WIN16_XAML_SKIP_AMBIGUOUS_EVENT");
        else signal_named(L"WIN16_XAML_SKIP_NONDEFAULT_EVENT");
        return;
    }
    if (InterlockedCompareExchange(&style_attempted,1,0) != 0) return;
    HSTRING name=NULL, text=NULL;
    reader *factory=NULL;
    if (GetEnvironmentVariableW(L"WIN16_XAML_CONFLICT_TEST",NULL,0)) {
        hr=WindowsCreateString(reader_name,(UINT32)(ARRAYSIZE(reader_name)-1),&name);
        if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_reader_iid,(void **)&factory);
        if (SUCCEEDED(hr)) hr=WindowsCreateString(markup,(UINT32)lstrlenW(markup),&text);
        if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,text,&style_alternate);
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_set_add(&style_set,service,diagnostics,button,&style_state);
    if (SUCCEEDED(hr)) {
        win16_xaml_button_style *duplicate=NULL;
        HRESULT second=win16_xaml_button_set_add(&style_set,service,diagnostics,button,&duplicate);
        if (second==S_FALSE && duplicate==style_state) signal_named(L"WIN16_XAML_DUPLICATE_EVENT");
        else signal_named(L"WIN16_XAML_FAILED_EVENT");
        HANDLE worker=CreateThread(NULL,0,test_wrong_thread,&style_set,0,NULL);
        if (worker) {
            DWORD result=0;
            if (WaitForSingleObject(worker,1000)==WAIT_OBJECT_0 && GetExitCodeThread(worker,&result) &&
                result==(DWORD)RPC_E_WRONG_THREAD) signal_named(L"WIN16_XAML_WRONG_THREAD_EVENT");
            else signal_named(L"WIN16_XAML_FAILED_EVENT");
            CloseHandle(worker);
        } else signal_named(L"WIN16_XAML_FAILED_EVENT");
        signal_named(L"WIN16_XAML_APPLIED_EVENT");
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,button,&style_target);
        style_index=style_state->property_index;
        style_expected=style_state->installed_style; IInspectable_AddRef(style_expected);
        style_button=button; IInspectable_AddRef(style_button);
        if (SUCCEEDED(hr)) style_timer=SetTimer(NULL,0,GetEnvironmentVariableW(L"WIN16_XAML_MULTI_TEST",NULL,0) ? 1200 : 350,restore_style_timer);
        if (style_timer && style_alternate && !SetTimer(NULL,0,100,change_style_timer)) {
            KillTimer(NULL,style_timer); style_timer=0;
        }
        if (!style_timer) {
            signal_named(L"WIN16_XAML_TIMER_FAILED_EVENT");
            if (SUCCEEDED(hr)) { DWORD error=GetLastError(); hr=HRESULT_FROM_WIN32(error ? error : ERROR_NOT_ENOUGH_MEMORY); }
            win16_xaml_button_set_remove(&style_set,service,diagnostics,style_state);
            win16_xaml_button_set_discard(&style_set); style_state=NULL;
            IInspectable_Release(style_button); style_button=NULL;
            IInspectable_Release(style_expected); style_expected=NULL;
            if (style_alternate) { IInspectable_Release(style_alternate); style_alternate=NULL; }
        }
    }
    if (factory) factory->vtable->release(factory);
    if (text) WindowsDeleteString(text);
    if (name) WindowsDeleteString(name);
    if (FAILED(hr) && style_alternate && !style_timer) { IInspectable_Release(style_alternate); style_alternate=NULL; }
    if (FAILED(hr)) signal_named(L"WIN16_XAML_FAILED_EVENT");
}

static HRESULT STDMETHODCALLTYPE callback_query(IVisualTreeServiceCallback *self, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    *out = NULL;
    if (IsEqualIID(iid,&IID_IMarshal) && callback_marshaler)
        return IUnknown_QueryInterface(callback_marshaler,iid,out);
    if (!IsEqualIID(iid,&IID_IUnknown) && !IsEqualIID(iid,&IID_IVisualTreeServiceCallback)) return E_NOINTERFACE;
    *out = self; return S_OK;
}
static ULONG STDMETHODCALLTYPE callback_ref(IVisualTreeServiceCallback *self) { (void)self; return 2; }
static ULONG STDMETHODCALLTYPE callback_unref(IVisualTreeServiceCallback *self) { (void)self; return 1; }
static void CALLBACK stop_reusable_session(HWND window, UINT message, UINT_PTR timer, DWORD time)
{
    (void)window; (void)message; (void)time;
    KillTimer(NULL,timer); reusable_timer=0;
    reusable_finished=TRUE;
    HRESULT hr=GetEnvironmentVariableW(L"WIN16_XAML_ADAPTER_TEST",NULL,0)
        ? win16_xaml_adapter_stop(&reusable_adapter) : win16_xaml_session_stop(&reusable_session);
    unsigned int matches=0,source=0,index=0;
    if (SUCCEEDED(hr) && reusable_button) hr=TestStyleSource(reusable_button,&matches,&source,&index);
    if (SUCCEEDED(hr) && reusable_button && matches==1 && source==BaseValueSourceDefault)
        signal_named(L"WIN16_XAML_SESSION_STOPPED_EVENT");
    else signal_named(L"WIN16_XAML_FAILED_EVENT");
    if (reusable_button) { IInspectable_Release(reusable_button); reusable_button=NULL; }
    if (GetEnvironmentVariableW(L"WIN16_XAML_DISCONNECT_TEST",NULL,0) && generic_site) {
        signal_named(L"WIN16_XAML_SITE_DISCONNECTING_EVENT");
        HRESULT result=IObjectWithSite_SetSite(generic_site,NULL);
        signal_named(SUCCEEDED(result) ? L"WIN16_XAML_SITE_DISCONNECTED_EVENT" : L"WIN16_XAML_FAILED_EVENT");
        IObjectWithSite_Release(generic_site); generic_site=NULL;
    }
}
static BOOL visit_button_ancestor(IInspectable *object, unsigned int depth, void *context)
{
    (void)context;
    HSTRING type=NULL;
    HRESULT hr=IInspectable_GetRuntimeClassName(object,&type);
    BOOL button=SUCCEEDED(hr) && type && button_class(WindowsGetStringRawBuffer(type,NULL));
    if (button) {
        if (!found_button) { found_button=object; IInspectable_AddRef(object); }
        InterlockedExchange(&button_seen,1); signal_named(L"WIN16_XAML_FOUND_EVENT");
        if (depth) signal_named(L"WIN16_XAML_DEEP_ANCESTOR_EVENT");
        if (!wcscmp(WindowsGetStringRawBuffer(type,NULL),L"Microsoft.UI.Xaml.Controls.Button"))
            try_winui_style(object);
    }
    if (type) WindowsDeleteString(type);
    return !button;
}
static HRESULT STDMETHODCALLTYPE callback_change(IVisualTreeServiceCallback *self, ParentChildRelation relation,
                                                  VisualElement element, VisualMutationType mutation)
{
    (void)self;
    InterlockedIncrement(&changes);
    WCHAR tag_prefix[100],tag_name[144];
    DWORD tag_length=GetEnvironmentVariableW(L"WIN16_XAML_CALLBACK_TAG",tag_prefix,ARRAYSIZE(tag_prefix));
    if (tag_length && tag_length<ARRAYSIZE(tag_prefix) && !callback_thread_tag) {
        swprintf(tag_name,ARRAYSIZE(tag_name),L"%ls-%lu",tag_prefix,(unsigned long)GetCurrentThreadId());
        callback_thread_tag=CreateEventW(NULL,TRUE,FALSE,tag_name);
    }
    signal_named(L"WIN16_XAML_TREE_EVENT");
    /* The template fixture needs a WinUI callback thread, not a Button event. */
    if (mutation==Add && diagnostics && element.Type &&
        !wcsncmp(element.Type,L"Microsoft.UI.Xaml.",18)) test_template_parse();
    if (mutation == Add && element.Type) fwprintf(stderr,L"type=%ls handle=%llu parent=%llu children=%u\n",element.Type,
        (unsigned long long)element.Handle,(unsigned long long)relation.Parent,element.NumChildren);
    if (mutation == Add && button_class(element.Type)) {
        InterlockedExchange(&button_seen,1);
        signal_named(L"WIN16_XAML_FOUND_EVENT");
        if (diagnostics && !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.Button")) {
            IInspectable *object = NULL;
            if (SUCCEEDED(IXamlDiagnostics_GetIInspectableFromHandle(diagnostics,element.Handle,&object))) {
                try_winui_style(object);
                IInspectable_Release(object);
            }
        }
    }
    if (mutation == Add && diagnostics && relation.Parent) {
        IInspectable *parent = NULL;
        HRESULT hr = IXamlDiagnostics_GetIInspectableFromHandle(diagnostics,relation.Parent,&parent);
        if (SUCCEEDED(hr) && parent && GetEnvironmentVariableW(L"WIN16_XAML_SESSION_TEST",NULL,0) && !reusable_finished) {
            if (!reusable_session.service) hr=win16_xaml_session_init(&reusable_session,service,diagnostics);
            IInspectable *applied=NULL;
            if (SUCCEEDED(hr)) hr=win16_xaml_session_observe(&reusable_session,parent,&applied);
            if (hr==S_FALSE) signal_named(L"WIN16_XAML_SESSION_NOCHANGE_EVENT");
            else if (FAILED(hr)) {
                signal_named(L"WIN16_XAML_SESSION_ERROR_EVENT");
                signal_named(hr==RPC_E_WRONG_THREAD ? L"WIN16_XAML_SESSION_WRONG_THREAD_EVENT" : L"WIN16_XAML_SESSION_OTHER_ERROR_EVENT");
                if (hr==E_NOINTERFACE) signal_named(L"WIN16_XAML_SESSION_NOINTERFACE_EVENT");
                else if (hr==E_INVALIDARG) signal_named(L"WIN16_XAML_SESSION_INVALID_EVENT");
                else if (hr==HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) signal_named(L"WIN16_XAML_SESSION_NOTFOUND_EVENT");
                else if (hr==HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW)) signal_named(L"WIN16_XAML_SESSION_DEPTH_EVENT");
            }
            if (applied) {
                if (!reusable_button) { reusable_button=applied; applied=NULL; }
                if (applied) IInspectable_Release(applied);
            }
            if (hr==S_OK) {
                signal_named(L"WIN16_XAML_SESSION_APPLIED_EVENT");
                if (!reusable_timer) reusable_timer=SetTimer(NULL,0,1200,stop_reusable_session);
                if (!reusable_timer) signal_named(L"WIN16_XAML_FAILED_EVENT");
            }
        } else if (SUCCEEDED(hr) && parent) (void)win16_xaml_visit_ancestors(parent,visit_button_ancestor,NULL);
        if (parent) IInspectable_Release(parent);
    }
    return S_OK;
}
static IVisualTreeServiceCallbackVtbl callback_vtable = {callback_query,callback_ref,callback_unref,callback_change};
static IVisualTreeServiceCallback callback = {&callback_vtable};
static UINT_PTR delayed_subscription;
static void subscription_complete(void *context,HRESULT result)
{
    (void)context;
    signal_named(SUCCEEDED(result) ? L"WIN16_XAML_SITE_READY_EVENT" : L"WIN16_XAML_SITE_FAILED_EVENT");
}
static void CALLBACK subscribe_later(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)window; (void)message; (void)time;
    KillTimer(NULL,timer); delayed_subscription=0;
    signal_named(L"WIN16_XAML_SUBSCRIBE_TIMER_EVENT");
    if (GetEnvironmentVariableW(L"WIN16_XAML_WORKER_SUBSCRIBE",NULL,0)) {
        HRESULT hr=callback_marshaler ? S_OK : CoCreateFreeThreadedMarshaler((IUnknown *)&callback,&callback_marshaler);
        HANDLE worker=NULL;
        if (SUCCEEDED(hr)) hr=win16_xaml_subscribe_async(service,&callback,subscription_complete,NULL,&worker);
        if (worker) CloseHandle(worker);
        if (FAILED(hr)) signal_named(L"WIN16_XAML_SITE_FAILED_EVENT");
        return;
    }
    HRESULT hr=service ? IVisualTreeService_AdviseVisualTreeChange(service,&callback) : E_FAIL;
    signal_named(SUCCEEDED(hr) ? L"WIN16_XAML_SITE_READY_EVENT" : L"WIN16_XAML_SITE_FAILED_EVENT");
}

static HRESULT STDMETHODCALLTYPE site_query(IObjectWithSite *self, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    *out = NULL;
    if (!IsEqualIID(iid,&IID_IUnknown) && !IsEqualIID(iid,&IID_IObjectWithSite)) return E_NOINTERFACE;
    *out = self; return S_OK;
}
static ULONG STDMETHODCALLTYPE site_ref(IObjectWithSite *self) { (void)self; return 2; }
static ULONG STDMETHODCALLTYPE site_unref(IObjectWithSite *self) { (void)self; return 1; }
static HRESULT STDMETHODCALLTYPE site_set(IObjectWithSite *self, IUnknown *site)
{
    (void)self;
    if (delayed_subscription) { KillTimer(NULL,delayed_subscription); delayed_subscription=0; }
    if (site && !site_thread_tag) {
        WCHAR prefix[100],name[144];
        DWORD length=GetEnvironmentVariableW(L"WIN16_XAML_THREAD_TAG",prefix,ARRAYSIZE(prefix));
        if (length && length<ARRAYSIZE(prefix)) {
            swprintf(name,ARRAYSIZE(name),L"%ls-%lu",prefix,(unsigned long)GetCurrentThreadId());
            site_thread_tag=CreateEventW(NULL,TRUE,FALSE,name);
        }
    }
    if (site) InterlockedIncrement(&site_seen);
    if (found_button) { IInspectable_Release(found_button); found_button = NULL; }
    if (service) {
        IVisualTreeService_UnadviseVisualTreeChange(service,&callback);
        IVisualTreeService_Release(service); service = NULL;
    }
    if (diagnostics) { IXamlDiagnostics_Release(diagnostics); diagnostics = NULL; }
    if (!site) return S_OK;
    HRESULT hr = IUnknown_QueryInterface(site,&IID_IVisualTreeService,(void **)&service);
    if (SUCCEEDED(hr)) hr = IUnknown_QueryInterface(site,&IID_IXamlDiagnostics,(void **)&diagnostics);
    if (SUCCEEDED(hr)) {
        signal_named(L"WIN16_XAML_SITE_CONNECTED_EVENT");
        if (GetEnvironmentVariableW(L"WIN16_XAML_DEFER_SUBSCRIBE",NULL,0)) {
            delayed_subscription=SetTimer(NULL,0,1,subscribe_later);
            if (delayed_subscription) signal_named(L"WIN16_XAML_SUBSCRIBE_SCHEDULED_EVENT");
            if (!delayed_subscription) { DWORD error=GetLastError(); hr=HRESULT_FROM_WIN32(error ? error : ERROR_NOT_ENOUGH_MEMORY); }
        } else {
            hr = IVisualTreeService_AdviseVisualTreeChange(service,&callback);
            if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_SITE_READY_EVENT");
            else signal_named(L"WIN16_XAML_SITE_FAILED_EVENT");
        }
        if (FAILED(hr)) { IVisualTreeService_Release(service); service = NULL; }
    }
    if (FAILED(hr) && service) { IVisualTreeService_Release(service); service = NULL; }
    if (FAILED(hr) && diagnostics) { IXamlDiagnostics_Release(diagnostics); diagnostics = NULL; }
    return hr;
}
static HRESULT STDMETHODCALLTYPE site_get(IObjectWithSite *self, REFIID iid, void **out)
{ (void)self; (void)iid; if (out) *out=NULL; return E_NOINTERFACE; }
static IObjectWithSiteVtbl site_vtable = {site_query,site_ref,site_unref,site_set,site_get};
static IObjectWithSite site_object = {&site_vtable};

static HRESULT STDMETHODCALLTYPE factory_query(IClassFactory *self, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    *out = NULL;
    if (!IsEqualIID(iid,&IID_IUnknown) && !IsEqualIID(iid,&IID_IClassFactory)) return E_NOINTERFACE;
    *out = self; return S_OK;
}
static ULONG STDMETHODCALLTYPE factory_ref(IClassFactory *self) { (void)self; return 2; }
static ULONG STDMETHODCALLTYPE factory_unref(IClassFactory *self) { (void)self; return 1; }
static HRESULT generic_connect(void *context,IVisualTreeService *new_service,IXamlDiagnostics *new_diagnostics,IObjectWithSite *site)
{
    (void)context; (void)site;
    WCHAR prefix[100],name[144];
    DWORD length=GetEnvironmentVariableW(L"WIN16_XAML_THREAD_TAG",prefix,ARRAYSIZE(prefix));
    if (length && length<ARRAYSIZE(prefix) && !site_thread_tag) {
        swprintf(name,ARRAYSIZE(name),L"%ls-%lu",prefix,(unsigned long)GetCurrentThreadId());
        site_thread_tag=CreateEventW(NULL,TRUE,FALSE,name);
    }
    if (service || diagnostics) return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
    if (GetEnvironmentVariableW(L"WIN16_XAML_ADAPTER_TEST",NULL,0)) {
        HRESULT hr=win16_xaml_adapter_connect(&reusable_adapter,new_service,new_diagnostics);
        if (FAILED(hr)) return hr;
    }
    service=new_service; diagnostics=new_diagnostics;
    IVisualTreeService_AddRef(service); IXamlDiagnostics_AddRef(diagnostics);
    InterlockedIncrement(&site_seen);
    signal_named(L"WIN16_XAML_SITE_CONNECTED_EVENT");
    return S_OK;
}
static void generic_disconnect(void *context)
{
    (void)context;
    if (GetEnvironmentVariableW(L"WIN16_XAML_ADAPTER_TEST",NULL,0)) win16_xaml_adapter_disconnect(&reusable_adapter);
    if (site_thread_tag) { CloseHandle(site_thread_tag); site_thread_tag=NULL; }
    if (diagnostics) { IXamlDiagnostics_Release(diagnostics); diagnostics=NULL; }
    if (service) { IVisualTreeService_Release(service); service=NULL; }
}
static HRESULT generic_change(void *context,ParentChildRelation relation,VisualElement element,VisualMutationType mutation)
{
    (void)context;
    if (!GetEnvironmentVariableW(L"WIN16_XAML_ADAPTER_TEST",NULL,0)) return callback_change(NULL,relation,element,mutation);
    signal_named(L"WIN16_XAML_TREE_EVENT");
    if (reusable_finished) return S_OK;
    IInspectable *applied=NULL;
    HRESULT hr=win16_xaml_adapter_change(&reusable_adapter,relation,element,mutation,&applied);
    if (hr==S_OK && applied) {
        if (!reusable_button) { reusable_button=applied; applied=NULL; }
        signal_named(L"WIN16_XAML_SESSION_APPLIED_EVENT");
        if (!reusable_timer) reusable_timer=SetTimer(NULL,0,1200,stop_reusable_session);
        if (!reusable_timer) signal_named(L"WIN16_XAML_FAILED_EVENT");
    } else if (FAILED(hr)) {
        signal_named(L"WIN16_XAML_SESSION_ERROR_EVENT");
        if (hr==E_NOINTERFACE) signal_named(L"WIN16_XAML_SESSION_NOINTERFACE_EVENT");
        else if (hr==E_INVALIDARG) signal_named(L"WIN16_XAML_SESSION_INVALID_EVENT");
        else if (hr==HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) signal_named(L"WIN16_XAML_SESSION_NOTFOUND_EVENT");
        else signal_named(L"WIN16_XAML_SESSION_OTHER_ERROR_EVENT");
    }
    if (applied) IInspectable_Release(applied);
    return S_OK;
}
static void generic_ready(void *context) { (void)context; signal_named(L"WIN16_XAML_SITE_READY_EVENT"); }
static void generic_failed(void *context,HRESULT error)
{ (void)context; (void)error; signal_named(L"WIN16_XAML_SITE_FAILED_EVENT"); }
static HRESULT STDMETHODCALLTYPE factory_create(IClassFactory *self, IUnknown *outer, REFIID iid, void **out)
{
    (void)self;
    if (outer) return CLASS_E_NOAGGREGATION;
    if (GetEnvironmentVariableW(L"WIN16_XAML_GENERIC_SITE_TEST",NULL,0)) {
        const win16_xaml_site_handlers handlers={generic_connect,generic_disconnect,generic_change,generic_ready,generic_failed};
        HRESULT result=GetEnvironmentVariableW(L"WIN16_XAML_ASYNC_SITE_TEST",NULL,0)
            ? win16_xaml_site_create_async(&handlers,NULL,iid,out)
            : win16_xaml_site_create(&handlers,NULL,iid,out);
        if (SUCCEEDED(result) && GetEnvironmentVariableW(L"WIN16_XAML_DISCONNECT_TEST",NULL,0))
            (void)IUnknown_QueryInterface((IUnknown *)*out,&IID_IObjectWithSite,(void **)&generic_site);
        return result;
    }
    return site_query(&site_object,iid,out);
}
static HRESULT STDMETHODCALLTYPE factory_lock(IClassFactory *self, BOOL lock) { (void)self; (void)lock; return S_OK; }
static IClassFactoryVtbl factory_vtable = {factory_query,factory_ref,factory_unref,factory_create,factory_lock};
static IClassFactory factory = {&factory_vtable};

__declspec(dllexport) HRESULT STDAPICALLTYPE DllGetClassObject(REFCLSID clsid, REFIID iid, void **out)
{
    if (GetEnvironmentVariableW(L"WIN16_XAML_FACTORY_TEST",NULL,0)) {
        const win16_xaml_site_handlers handlers={generic_connect,generic_disconnect,generic_change,generic_ready,generic_failed};
        return win16_xaml_get_class_object(clsid,&tap_clsid,&handlers,NULL,iid,out);
    }
    return IsEqualCLSID(clsid,&tap_clsid) ? factory_query(&factory,iid,out) : CLASS_E_CLASSNOTAVAILABLE;
}
__declspec(dllexport) LONG WINAPI TestSawButton(void) { return InterlockedCompareExchange(&button_seen,0,0); }
__declspec(dllexport) LONG WINAPI TestSiteSeen(void) { return InterlockedCompareExchange(&site_seen,0,0); }
__declspec(dllexport) LONG WINAPI TestChanges(void) { return InterlockedCompareExchange(&changes,0,0); }
__declspec(dllexport) HRESULT WINAPI TestGetButton(IInspectable **object)
{
    if (!object) return E_POINTER;
    *object = found_button;
    if (!*object) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    IInspectable_AddRef(*object);
    return S_OK;
}
__declspec(dllexport) HRESULT WINAPI TestHitTest(unsigned int *total, unsigned int *buttons)
{
    if (!diagnostics || !total || !buttons) return E_INVALIDARG;
    RECT area = {0,0,320,200};
    InstanceHandle *handles = NULL;
    *total = 0; *buttons = 0;
    HRESULT hr = IXamlDiagnostics_HitTest(diagnostics,area,total,&handles);
    if (SUCCEEDED(hr)) {
        for (unsigned int i=0;i<*total;i++) {
            IInspectable *object = NULL;
            HSTRING type = NULL;
            HRESULT item = IXamlDiagnostics_GetIInspectableFromHandle(diagnostics,handles[i],&object);
            if (SUCCEEDED(item)) item = IInspectable_GetRuntimeClassName(object,&type);
            if (SUCCEEDED(item) && button_class(WindowsGetStringRawBuffer(type,NULL))) {
                ++*buttons;
                if (!found_button) { found_button = object; IInspectable_AddRef(found_button); }
                InterlockedExchange(&button_seen,1);
            }
            if (type) WindowsDeleteString(type);
            if (object) IInspectable_Release(object);
        }
    }
    CoTaskMemFree(handles);
    return hr;
}
__declspec(dllexport) HRESULT WINAPI TestRoundtrip(IInspectable *object)
{
    if (!diagnostics || !object) return E_INVALIDARG;
    InstanceHandle handle = 0;
    IInspectable *result = NULL;
    IUnknown *a = NULL, *b = NULL;
    HRESULT hr = IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&handle);
    if (SUCCEEDED(hr)) fwprintf(stderr,L"button_handle=%llu\n",(unsigned long long)handle);
    if (SUCCEEDED(hr)) hr = IXamlDiagnostics_GetIInspectableFromHandle(diagnostics,handle,&result);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(object,&IID_IUnknown,(void **)&a);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(result,&IID_IUnknown,(void **)&b);
    if (SUCCEEDED(hr) && a != b) hr = E_FAIL;
    if (a) IUnknown_Release(a);
    if (b) IUnknown_Release(b);
    if (result) IInspectable_Release(result);
    return hr;
}
static void free_source_info(SourceInfo *info)
{
    SysFreeString(info->FileName);
    SysFreeString(info->Hash);
}
__declspec(dllexport) HRESULT WINAPI TestStyleSource(IInspectable *object, unsigned int *matches, unsigned int *source, unsigned int *index)
{
    if (!service || !diagnostics || !object || !matches || !source || !index) return E_INVALIDARG;
    InstanceHandle handle = 0;
    PropertyChainSource *sources = NULL;
    PropertyChainValue *values = NULL;
    unsigned int source_count = 0, value_count = 0;
    *matches = 0; *source = 0; *index = 0;
    HRESULT hr = IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&handle);
    if (SUCCEEDED(hr)) hr = IVisualTreeService_GetPropertyValuesChain(service,handle,&source_count,&sources,&value_count,&values);
    if (SUCCEEDED(hr)) {
        for (unsigned int i=0;i<value_count;i++) {
            if (values[i].PropertyName && !wcscmp(values[i].PropertyName,L"Style") && !values[i].Overridden) {
                ++*matches;
                *index = values[i].Index;
                if (values[i].PropertyChainIndex < source_count) {
                    *source = (unsigned int)sources[values[i].PropertyChainIndex].Source;
                    if (*source==BaseValueSourceLocal) {
                        if (values[i].MetadataBits & IsValueBindingExpression) signal_named(L"WIN16_XAML_LOCAL_BINDING_EVENT");
                        if (values[i].MetadataBits & IsValueHandle) signal_named(L"WIN16_XAML_LOCAL_HANDLE_EVENT");
                        if (values[i].MetadataBits & IsValueHandleAndEvaluatedValue) signal_named(L"WIN16_XAML_LOCAL_EVALUATED_EVENT");
                        if (values[i].MetadataBits & IsValueNull) signal_named(L"WIN16_XAML_LOCAL_NULL_EVENT");
                    }
                }
            }
        }
    }
    for (unsigned int i=0;i<source_count;i++) {
        SysFreeString(sources[i].TargetType); SysFreeString(sources[i].Name); free_source_info(&sources[i].SrcInfo);
    }
    for (unsigned int i=0;i<value_count;i++) {
        SysFreeString(values[i].Type); SysFreeString(values[i].DeclaringType); SysFreeString(values[i].ValueType);
        SysFreeString(values[i].ItemType); SysFreeString(values[i].Value); SysFreeString(values[i].PropertyName);
    }
    CoTaskMemFree(sources); CoTaskMemFree(values);
    return hr;
}
__declspec(dllexport) HRESULT WINAPI TestClearStyle(IInspectable *object, unsigned int index)
{
    if (!service || !diagnostics || !object) return E_INVALIDARG;
    InstanceHandle handle = 0;
    HRESULT hr = IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&handle);
    if (SUCCEEDED(hr)) hr = IVisualTreeService_ClearProperty(service,handle,index);
    return hr;
}
__declspec(dllexport) void WINAPI TestStop(void) { site_set(&site_object,NULL); }
