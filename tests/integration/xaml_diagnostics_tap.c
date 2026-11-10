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
#include "lib-win32/win16-control-xaml/radio_template.h"
#include "lib-win32/win16-control-xaml/textbox_template.h"
#include "lib-win32/win16-control-xaml/combo_template.h"
#include "lib-win32/win16-control-xaml/scroll_template.h"
#include "lib-win32/win16-control-xaml/list_template.h"
#include "lib-win32/win16-control-xaml/progress_template.h"
#include "lib-win32/win16-control-xaml/toggle_template.h"
#include "lib-win32/win16-control-xaml/slider_template.h"
#include "lib-win32/win16-control-xaml/toggle_button_template.h"
#include "lib-win32/win16-control-xaml/menu_presenter_template.h"
#include "lib-win32/win16-control-xaml/menu_item_template.h"
#include "lib-win32/win16-control-xaml/toggle_menu_template.h"
#include "lib-win32/win16-control-xaml/submenu_template.h"
#include "lib-win32/win16-control-xaml/menu_separator_template.h"
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
    void (STDMETHODCALLTYPE *unused_find_name[23])(void);
    HRESULT (STDMETHODCALLTYPE *find_name)(winui_element *,HSTRING,IInspectable **);
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
    void (STDMETHODCALLTYPE *unused_visibility[13])(void);
    HRESULT (STDMETHODCALLTYPE *get_visibility)(test_ui *,INT32 *);
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

static HRESULT test_radio_apply(void)
{
    static const wchar_t runtime[]=L"Microsoft.UI.Xaml.Controls.RadioButton";
    static const wchar_t window_runtime[]=L"Microsoft.UI.Xaml.Window";
    static const wchar_t tree_runtime[]=L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    HSTRING name=NULL; IInspectable *radio=NULL,*window_object=NULL,*element=NULL,*root=NULL,*box=NULL,*mark=NULL,*dep=NULL;
    test_window *window=NULL; test_control *control=NULL; test_toggle *toggle=NULL; test_ui *mark_ui=NULL;
    winui_tree *tree=NULL; win16_xaml_adapter adapter={0};
    __x_ABI_CWindows_CFoundation_CIPropertyValueStatics *properties=NULL;
    VisualElement info={0}; ParentChildRelation relation={0}; unsigned int source=0,index=0;
    HRESULT hr=WindowsCreateString(runtime,(UINT32)(ARRAYSIZE(runtime)-1),&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&radio);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(window_runtime,(UINT32)(ARRAYSIZE(window_runtime)-1),&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(radio,&test_ui_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,element);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) { info.Type=(BSTR)runtime; relation.Parent=1; hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,radio,&info.Handle); }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,radio,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(radio,&winui_control_iid,(void **)&control);
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(tree_runtime,(UINT32)(ARRAYSIZE(tree_runtime)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(radio,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (dep) { IInspectable_Release(dep); dep=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&box);
    if (dep) { IInspectable_Release(dep); dep=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(box,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,1,&mark);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(mark,&test_ui_iid,(void **)&mark_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Windows.Foundation.PropertyValue",32,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics,(void **)&properties);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(radio,&test_toggle_iid,(void **)&toggle);
    for (unsigned int i=0;SUCCEEDED(hr) && i<2;i++) {
        IInspectable *boxed=NULL; IUnknown *reference=NULL; double opacity=-1;
        hr=__x_ABI_CWindows_CFoundation_CIPropertyValueStatics_CreateBoolean(properties,(boolean)i,&boxed);
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(boxed,&IID___FIReference_1_boolean,(void **)&reference);
        if (SUCCEEDED(hr)) hr=toggle->vtable->put_checked(toggle,reference);
        if (SUCCEEDED(hr)) hr=mark_ui->vtable->get_opacity(mark_ui,&opacity);
        if (SUCCEEDED(hr) && opacity!=(double)i) hr=E_FAIL;
        if (reference) IUnknown_Release(reference);
        if (boxed) IInspectable_Release(boxed);
    }
    if (adapter.service) { HRESULT removed=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=removed; }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,radio,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    /* A local style owned by an application must not be replaced. */
    win16_xaml_button_style external={0};
    if (SUCCEEDED(hr)) hr=win16_xaml_button_apply(service,diagnostics,radio,&external);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,radio,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (external.button) { HRESULT removed=win16_xaml_button_remove(service,diagnostics,&external); if(SUCCEEDED(hr))hr=removed; }
    if (external.button) win16_xaml_button_discard(&external);
    if (toggle) toggle->vtable->release(toggle);
    if (properties) __x_ABI_CWindows_CFoundation_CIPropertyValueStatics_Release(properties);
    if (mark_ui) mark_ui->vtable->release(mark_ui);
    if (dep) IInspectable_Release(dep);
    if (mark) IInspectable_Release(mark);
    if (box) IInspectable_Release(box);
    if (root) IInspectable_Release(root);
    if (tree) tree->vtable->release(tree);
    if (control) control->vtable->release(control);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) IInspectable_Release(element);
    if (window_object) IInspectable_Release(window_object);
    if (radio) IInspectable_Release(radio);
    if (name) WindowsDeleteString(name);
    return hr;
}

static HRESULT test_radio_group(void)
{
    static const wchar_t reader_name[]=L"Microsoft.UI.Xaml.Markup.XamlReader";
    static const wchar_t tree_name[]=L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    static const wchar_t window_name[]=L"Microsoft.UI.Xaml.Window";
    static const wchar_t markup[]=L"<StackPanel xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation'><RadioButton/><RadioButton/></StackPanel>";
    HSTRING name=NULL,text=NULL; reader *factory=NULL; winui_tree *tree=NULL; test_window *window=NULL;
    IInspectable *panel=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*items[2]={NULL,NULL};
    test_toggle *toggles[2]={NULL,NULL}; win16_xaml_adapter adapter={0};
    __x_ABI_CWindows_CFoundation_CIPropertyValueStatics *properties=NULL;
    HRESULT hr=WindowsCreateString(reader_name,(UINT32)(ARRAYSIZE(reader_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_reader_iid,(void **)&factory);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&text);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,text,&panel);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(window_name,(UINT32)(ARRAYSIZE(window_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(panel,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(tree_name,(UINT32)(ARRAYSIZE(tree_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(panel,&winui_dependency_iid,(void **)&dep);
    for (int i=0;SUCCEEDED(hr) && i<2;i++) hr=tree->vtable->get_child(tree,dep,i,&items[i]);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    for (unsigned int i=0;SUCCEEDED(hr) && i<2;i++) {
        VisualElement info={0}; ParentChildRelation relation={0}; relation.Parent=1;
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.RadioButton";
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,items[i],&info.Handle);
        if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(items[i],&test_toggle_iid,(void **)&toggles[i]);
    }
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Windows.Foundation.PropertyValue",32,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics,(void **)&properties);
    IInspectable *boxed=NULL; IUnknown *value=NULL,*first=NULL;
    __x_ABI_CWindows_CFoundation_CIPropertyValue *first_value=NULL; boolean first_checked=1;
    if (SUCCEEDED(hr)) hr=__x_ABI_CWindows_CFoundation_CIPropertyValueStatics_CreateBoolean(properties,1,&boxed);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(boxed,&IID___FIReference_1_boolean,(void **)&value);
    if (SUCCEEDED(hr)) hr=toggles[0]->vtable->put_checked(toggles[0],value);
    if (SUCCEEDED(hr)) hr=toggles[1]->vtable->put_checked(toggles[1],value);
    if (SUCCEEDED(hr)) hr=toggles[0]->vtable->get_checked(toggles[0],&first);
    if (SUCCEEDED(hr) && !first) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IUnknown_QueryInterface(first,&IID___x_ABI_CWindows_CFoundation_CIPropertyValue,(void **)&first_value);
    if (SUCCEEDED(hr)) hr=__x_ABI_CWindows_CFoundation_CIPropertyValue_GetBoolean(first_value,&first_checked);
    if (SUCCEEDED(hr) && first_checked) hr=E_FAIL;
    if (first_value) __x_ABI_CWindows_CFoundation_CIPropertyValue_Release(first_value);
    if (first) IUnknown_Release(first);
    if (value) IUnknown_Release(value);
    if (boxed) IInspectable_Release(boxed);
    if (properties) __x_ABI_CWindows_CFoundation_CIPropertyValueStatics_Release(properties);
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    for (int i=1;i>=0;i--) { if(toggles[i])toggles[i]->vtable->release(toggles[i]); if(items[i])IInspectable_Release(items[i]); }
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (panel) IInspectable_Release(panel);
    if (factory) factory->vtable->release(factory);
    if (text) WindowsDeleteString(text);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_textbox test_textbox;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_textbox *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_textbox *);
    ULONG (STDMETHODCALLTYPE *release)(test_textbox *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_textbox *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_textbox *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_textbox *, TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_text)(test_textbox *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *put_text)(test_textbox *, HSTRING);
    HRESULT (STDMETHODCALLTYPE *get_selected_text)(test_textbox *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *put_selected_text)(test_textbox *, HSTRING);
    HRESULT (STDMETHODCALLTYPE *get_selection_length)(test_textbox *, INT32 *);
    HRESULT (STDMETHODCALLTYPE *put_selection_length)(test_textbox *, INT32);
    HRESULT (STDMETHODCALLTYPE *get_selection_start)(test_textbox *, INT32 *);
    HRESULT (STDMETHODCALLTYPE *put_selection_start)(test_textbox *, INT32);
    void (STDMETHODCALLTYPE *unused[16])(void);
    HRESULT (STDMETHODCALLTYPE *get_header)(test_textbox *, IInspectable **);
    HRESULT (STDMETHODCALLTYPE *put_header)(test_textbox *, IInspectable *);
    void (STDMETHODCALLTYPE *unused_header[2])(void);
    HRESULT (STDMETHODCALLTYPE *get_placeholder)(test_textbox *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *put_placeholder)(test_textbox *, HSTRING);
} test_textbox_vtable;
struct test_textbox { const test_textbox_vtable *vtable; };
static const IID test_textbox_iid = {0x873af7c2,0xab89,0x5d76,{0x8d,0xbe,0x3d,0x63,0x25,0x66,0x9d,0xf5}};

static HRESULT test_textbox_candidate(void)
{
    static const wchar_t textbox_name[]=L"Microsoft.UI.Xaml.Controls.TextBox";
    static const wchar_t window_name[]=L"Microsoft.UI.Xaml.Window";
    HSTRING name=NULL,text=NULL,actual=NULL,replacement=NULL;
    IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*header=NULL,*header_value=NULL,*border=NULL,*grid=NULL,*placeholder=NULL,*boxed_header=NULL;
    test_textbox *box=NULL; test_window *window=NULL; test_control *control=NULL;
    winui_tree *tree=NULL; winui_vsm *manager=NULL; test_ui *placeholder_ui=NULL;
    content_interface *header_presenter=NULL;
    __x_ABI_CWindows_CFoundation_CIPropertyValueStatics *properties=NULL;
    __x_ABI_CWindows_CFoundation_CIPropertyValue *header_property=NULL;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    unsigned int source=0,index=0;
    HRESULT hr=WindowsCreateString(textbox_name,(UINT32)(ARRAYSIZE(textbox_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(window_name,(UINT32)(ARRAYSIZE(window_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_textbox_iid,(void **)&box);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"alpha beta",10,&text);
    HSTRING hint=NULL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Hint",4,&hint);
    if (SUCCEEDED(hr)) hr=box->vtable->put_placeholder(box,hint);
    HSTRING header_text=NULL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Header",6,&header_text);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Windows.Foundation.PropertyValue",32,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics,(void **)&properties);
    if (SUCCEEDED(hr)) hr=__x_ABI_CWindows_CFoundation_CIPropertyValueStatics_CreateString(properties,header_text,&boxed_header);
    if (SUCCEEDED(hr)) hr=box->vtable->put_header(box,boxed_header);
    if (SUCCEEDED(hr)) hr=box->vtable->put_text(box,text);
    if (SUCCEEDED(hr)) hr=box->vtable->put_selection_start(box,2);
    if (SUCCEEDED(hr)) hr=box->vtable->put_selection_length(box,3);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) { info.Type=(BSTR)textbox_name; relation.Parent=1; hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle); }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (dep) { IInspectable_Release(dep); dep=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&header);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(header,&winui_presenter_iid,(void **)&header_presenter);
    if (SUCCEEDED(hr)) hr=header_presenter->vtable->get_content(header_presenter,&header_value);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(header_value,&property_value_iid,(void **)&header_property);
    HSTRING displayed_header=NULL;
    if (SUCCEEDED(hr)) hr=__x_ABI_CWindows_CFoundation_CIPropertyValue_GetString(header_property,&displayed_header);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(displayed_header,NULL),L"Header")) hr=E_FAIL;
    if (displayed_header) WindowsDeleteString(displayed_header);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,1,&border);
    if (dep) { IInspectable_Release(dep); dep=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(border,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&grid);
    if (dep) { IInspectable_Release(dep); dep=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(grid,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,1,&placeholder);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(placeholder,&test_ui_iid,(void **)&placeholder_ui);
    INT32 visibility=-1;
    if (SUCCEEDED(hr)) hr=placeholder_ui->vtable->get_visibility(placeholder_ui,&visibility);
    if (SUCCEEDED(hr) && visibility!=1) hr=E_FAIL; /* Collapsed when text exists. */
    INT32 start=-1,length=-1;
    if (SUCCEEDED(hr)) hr=box->vtable->get_text(box,&actual);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(actual,NULL),L"alpha beta")) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=box->vtable->get_selection_start(box,&start);
    if (SUCCEEDED(hr)) hr=box->vtable->get_selection_length(box,&length);
    if (SUCCEEDED(hr) && (start!=2 || length!=3)) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"XYZ",3,&replacement);
    if (SUCCEEDED(hr)) hr=box->vtable->put_selected_text(box,replacement);
    if (actual) { WindowsDeleteString(actual); actual=NULL; }
    if (SUCCEEDED(hr)) hr=box->vtable->get_text(box,&actual);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(actual,NULL),L"alXYZ beta")) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=box->vtable->put_text(box,NULL);
    if (SUCCEEDED(hr)) hr=placeholder_ui->vtable->get_visibility(placeholder_ui,&visibility);
    if (SUCCEEDED(hr) && visibility!=0) hr=E_FAIL; /* Visible when empty. */
    if (SUCCEEDED(hr)) hr=box->vtable->put_text(box,text);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.VisualStateManager",36,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    static const wchar_t *states[]={L"Disabled",L"PointerOver",L"Focused",L"Normal"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        HSTRING state=NULL; boolean changed=0;
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (actual) { WindowsDeleteString(actual); actual=NULL; }
    if (SUCCEEDED(hr)) hr=box->vtable->get_text(box,&actual);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(actual,NULL),L"alpha beta")) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    win16_xaml_button_style external={0};
    if (SUCCEEDED(hr)) hr=win16_xaml_button_apply(service,diagnostics,object,&external);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (external.button) { HRESULT removed=win16_xaml_button_remove(service,diagnostics,&external); if(SUCCEEDED(hr))hr=removed; }
    if (external.button) win16_xaml_button_discard(&external);
    if (actual) WindowsDeleteString(actual);
    if (placeholder_ui) placeholder_ui->vtable->release(placeholder_ui);
    if (dep) IInspectable_Release(dep);
    if (placeholder) IInspectable_Release(placeholder);
    if (header_property) __x_ABI_CWindows_CFoundation_CIPropertyValue_Release(header_property);
    if (header_presenter) header_presenter->vtable->release(header_presenter);
    if (header_value) IInspectable_Release(header_value);
    if (header) IInspectable_Release(header);
    if (boxed_header) IInspectable_Release(boxed_header);
    if (properties) __x_ABI_CWindows_CFoundation_CIPropertyValueStatics_Release(properties);
    if (grid) IInspectable_Release(grid);
    if (border) IInspectable_Release(border);
    if (root) IInspectable_Release(root);
    if (tree) tree->vtable->release(tree);
    if (manager) manager->vtable->release(manager);
    if (control) control->vtable->release(control);
    if (box) box->vtable->release(box);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (replacement) WindowsDeleteString(replacement);
    if (hint) WindowsDeleteString(hint);
    if (header_text) WindowsDeleteString(header_text);
    if (text) WindowsDeleteString(text);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_combo test_combo;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_combo *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_combo *);
    ULONG (STDMETHODCALLTYPE *release)(test_combo *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_combo *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_combo *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_combo *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_open)(test_combo *,boolean *);
    HRESULT (STDMETHODCALLTYPE *put_open)(test_combo *,boolean);
    HRESULT (STDMETHODCALLTYPE *get_editable)(test_combo *,boolean *);
    HRESULT (STDMETHODCALLTYPE *put_editable)(test_combo *,boolean);
    void (STDMETHODCALLTYPE *unused_text[20])(void);
    HRESULT (STDMETHODCALLTYPE *get_text)(test_combo *,HSTRING *);
} test_combo_vtable;
struct test_combo { const test_combo_vtable *vtable; };
static const IID combo_iid={0xc77da58b,0x4fd7,0x51e0,{0xa4,0x31,0xf8,0x46,0x58,0xa8,0x3e,0x9e}};
typedef struct test_selector test_selector;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_selector *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_selector *);
    ULONG (STDMETHODCALLTYPE *release)(test_selector *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_selector *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_selector *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_selector *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_index)(test_selector *,INT32 *);
    HRESULT (STDMETHODCALLTYPE *put_index)(test_selector *,INT32);
} test_selector_vtable;
struct test_selector { const test_selector_vtable *vtable; };
static const IID selector_iid={0x8f7e2159,0xe61d,0x576f,{0x84,0x76,0xf8,0x3f,0xde,0x3d,0x68,0x9e}};
typedef struct test_popup test_popup;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_popup *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_popup *);
    ULONG (STDMETHODCALLTYPE *release)(test_popup *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_popup *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_popup *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_popup *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[2])(void);
    HRESULT (STDMETHODCALLTYPE *get_open)(test_popup *,boolean *);
} test_popup_vtable;
struct test_popup { const test_popup_vtable *vtable; };
static const IID popup_iid={0x4e3ab19d,0x2f95,0x579c,{0x95,0x35,0x90,0x6c,0x58,0x62,0x94,0x37}};
typedef struct test_layout test_layout;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_layout *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_layout *);
    ULONG (STDMETHODCALLTYPE *release)(test_layout *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_layout *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_layout *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_layout *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[214])(void);
    HRESULT (STDMETHODCALLTYPE *update_layout)(test_layout *);
} test_layout_vtable;
struct test_layout { const test_layout_vtable *vtable; };
typedef struct test_item_mapping test_item_mapping;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_item_mapping *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_item_mapping *);
    ULONG (STDMETHODCALLTYPE *release)(test_item_mapping *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_item_mapping *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_item_mapping *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_item_mapping *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[3])(void);
    HRESULT (STDMETHODCALLTYPE *container_from_index)(test_item_mapping *,INT32,IInspectable **);
} test_item_mapping_vtable;
struct test_item_mapping { const test_item_mapping_vtable *vtable; };
static const IID item_mapping_iid={0xb3e03a32,0xea83,0x5626,{0xbf,0xda,0xd3,0x27,0xa1,0xc7,0x55,0x0c}};

static HRESULT test_combo_item_states(IInspectable *item,winui_tree *tree,test_control *control,INT32 index)
{
    static const wchar_t type_name[]=L"Microsoft.UI.Xaml.VisualStateManager";
    HSTRING name=NULL; winui_vsm *manager=NULL;
    IInspectable *dep=NULL,*root=NULL,*root_dep=NULL,*content=NULL;
    HRESULT hr=WindowsCreateString(type_name,(UINT32)(ARRAYSIZE(type_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(item,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_dependency_iid,(void **)&root_dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,root_dep,0,&content);
    test_presenter *presenter=NULL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(content,&winui_presenter_iid,(void **)&presenter);
    content_interface *content_view=NULL; IInspectable *content_value=NULL;
    __x_ABI_CWindows_CFoundation_CIPropertyValue *property=NULL; HSTRING content_text=NULL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(content,&winui_presenter_iid,(void **)&content_view);
    if (SUCCEEDED(hr)) hr=content_view->vtable->get_content(content_view,&content_value);
    if (SUCCEEDED(hr) && !content_value) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(content_value,&property_value_iid,(void **)&property);
    if (SUCCEEDED(hr)) hr=__x_ABI_CWindows_CFoundation_CIPropertyValue_GetString(property,&content_text);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(content_text,NULL),index==0 ? L"First" : L"Second")) hr=E_FAIL;
    static const wchar_t *states[]={L"Selected",L"Disabled"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        HSTRING state=NULL; boolean changed=0; IInspectable *foreground=NULL; test_brush *brush=NULL; test_color color={0};
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=presenter->vtable->get_foreground(presenter,&foreground);
        if (SUCCEEDED(hr) && !foreground) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(foreground,&brush_iid,(void **)&brush);
        if (SUCCEEDED(hr)) hr=brush->vtable->get_color(brush,&color);
        BYTE expected=i==0 ? 255 : 128;
        if (SUCCEEDED(hr) && (color.r!=expected || color.g!=expected || color.b!=expected)) hr=E_FAIL;
        if (brush) brush->vtable->release(brush);
        if (foreground) IInspectable_Release(foreground);
        if (state) WindowsDeleteString(state);
    }
    if (presenter) presenter->vtable->release(presenter);
    if (content_text) WindowsDeleteString(content_text);
    if (property) __x_ABI_CWindows_CFoundation_CIPropertyValue_Release(property);
    if (content_value) IInspectable_Release(content_value);
    if (content_view) content_view->vtable->release(content_view);
    if (content) IInspectable_Release(content);
    if (root_dep) IInspectable_Release(root_dep);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (manager) manager->vtable->release(manager);
    if (name) WindowsDeleteString(name);
    return hr;
}

static HRESULT test_combo_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<ComboBox xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml'><x:String>First</x:String><x:String>Second</x:String></ComboBox>";
    HSTRING text=NULL,name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*popup_object=NULL;
    test_window *window=NULL; test_control *control=NULL; test_combo *combo=NULL; test_selector *selector=NULL;
    test_popup *popup=NULL; winui_tree *tree=NULL; winui_element *element=NULL,*root_element=NULL;
    BOOL baseline=GetEnvironmentVariableW(L"WIN16_XAML_COMBO_BASELINE",NULL,0)!=0;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    unsigned int source=0,property_index=0;
    (void)style;
    HRESULT hr=WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&text);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,text,&object);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_CONTENT_TEXT_MATCH_EVENT");
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr) && !baseline) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr) && !baseline) {
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.ComboBox"; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr) && !baseline) {
        hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (hr==S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr) && !baseline) hr=win16_xaml_button_source(service,diagnostics,object,&source,&property_index);
    if (SUCCEEDED(hr) && !baseline && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_STATES_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&selector_iid,(void **)&selector);
    if (SUCCEEDED(hr)) hr=selector->vtable->put_index(selector,1);
    INT32 index=-1;
    if (SUCCEEDED(hr)) hr=selector->vtable->get_index(selector,&index);
    if (SUCCEEDED(hr) && index!=1) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_BINDING_EVENT");
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"ContentPresenter",16,&name);
    IInspectable *presenter_object=NULL,*presented=NULL;
    content_interface *presenter=NULL;
    __x_ABI_CWindows_CFoundation_CIPropertyValue *presented_value=NULL;
    HSTRING presented_text=NULL;
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&presenter_object);
    if (SUCCEEDED(hr) && !presenter_object) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(presenter_object,&winui_presenter_iid,(void **)&presenter);
    if (SUCCEEDED(hr)) hr=presenter->vtable->get_content(presenter,&presented);
    if (SUCCEEDED(hr) && !presented) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(presented,&property_value_iid,(void **)&presented_value);
    if (SUCCEEDED(hr)) hr=__x_ABI_CWindows_CFoundation_CIPropertyValue_GetString(presented_value,&presented_text);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(presented_text,NULL),L"Second")) hr=E_FAIL;
    if (presented_text) WindowsDeleteString(presented_text);
    if (presented_value) __x_ABI_CWindows_CFoundation_CIPropertyValue_Release(presented_value);
    if (presented) IInspectable_Release(presented);
    if (presenter) presenter->vtable->release(presenter);
    if (presenter_object) IInspectable_Release(presenter_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Popup",5,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&popup_object);
    if (SUCCEEDED(hr) && !popup_object) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_HANDLE_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(popup_object,&popup_iid,(void **)&popup);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_EVALUATED_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&combo_iid,(void **)&combo);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_NULL_EVENT");
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=combo->vtable->put_open(combo,1);
    if (FAILED(hr)) {
        WCHAR path[MAX_PATH];
        DWORD length=GetEnvironmentVariableW(L"WIN16_XAML_COMBO_RESULT_FILE",path,ARRAYSIZE(path));
        if (length && length<ARRAYSIZE(path)) {
            HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,NULL);
            if (file!=INVALID_HANDLE_VALUE) {
                DWORD written=0; (void)WriteFile(file,&hr,sizeof(hr),&written,NULL); CloseHandle(file);
            }
        }
    }
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_BITMAP_OK_EVENT");
    boolean opened=0;
    if (SUCCEEDED(hr)) hr=popup->vtable->get_open(popup,&opened);
    if (SUCCEEDED(hr) && !opened) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    test_item_mapping *mapping=NULL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&item_mapping_iid,(void **)&mapping);
    for (INT32 i=0;SUCCEEDED(hr) && i<2;i++) {
        IInspectable *item=NULL;
        hr=mapping->vtable->container_from_index(mapping,i,&item);
        if (SUCCEEDED(hr) && !item) hr=E_FAIL;
        if (SUCCEEDED(hr)) {
            unsigned int item_source=0,item_index=0;
            hr=win16_xaml_button_source(service,diagnostics,item,&item_source,&item_index);
            if (SUCCEEDED(hr)) signal_named(item_source==BaseValueSourceDefault
                ? L"WIN16_XAML_SESSION_APPLIED_EVENT" : L"WIN16_XAML_SESSION_NOCHANGE_EVENT");
            if (SUCCEEDED(hr) && !baseline) {
                VisualElement item_info={0}; ParentChildRelation item_relation={0};
                item_info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.ComboBoxItem"; item_relation.Parent=1;
                hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,item,&item_info.Handle);
                if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,item_relation,item_info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
                if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,item,&item_source,&item_index);
                if (SUCCEEDED(hr) && item_source!=BaseValueSourceLocal) hr=E_FAIL;
                test_control *item_control=NULL;
                if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(item,&winui_control_iid,(void **)&item_control);
                boolean item_applied=0;
                if (SUCCEEDED(hr)) hr=item_control->vtable->apply_template(item_control,&item_applied);
                if (SUCCEEDED(hr) && !item_applied) hr=E_FAIL;
                if (SUCCEEDED(hr)) hr=test_combo_item_states(item,tree,item_control,i);
                if (item_control) item_control->vtable->release(item_control);
            }
        }
        if (item) IInspectable_Release(item);
    }
    if (mapping) mapping->vtable->release(mapping);
    if (SUCCEEDED(hr)) hr=combo->vtable->put_open(combo,0);
    if (SUCCEEDED(hr)) hr=combo->vtable->put_editable(combo,1);
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"EditableText",12,&name);
    IInspectable *editable=NULL;
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&editable);
    if (SUCCEEDED(hr) && !editable) hr=E_FAIL;
    test_ui *editable_ui=NULL; test_textbox *editable_box=NULL; HSTRING edit_text=NULL,combo_text=NULL;
    INT32 edit_visibility=-1;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(editable,&test_ui_iid,(void **)&editable_ui);
    if (SUCCEEDED(hr)) hr=editable_ui->vtable->get_visibility(editable_ui,&edit_visibility);
    if (SUCCEEDED(hr) && edit_visibility!=0) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(editable,&test_textbox_iid,(void **)&editable_box);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Edited",6,&edit_text);
    if (SUCCEEDED(hr)) hr=editable_box->vtable->put_text(editable_box,edit_text);
    if (SUCCEEDED(hr)) hr=combo->vtable->get_text(combo,&combo_text);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(combo_text,NULL),L"Edited")) hr=E_FAIL;
    if (combo_text) WindowsDeleteString(combo_text);
    if (edit_text) WindowsDeleteString(edit_text);
    if (editable_box) editable_box->vtable->release(editable_box);
    if (editable_ui) editable_ui->vtable->release(editable_ui);
    if (editable) IInspectable_Release(editable);
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (SUCCEEDED(hr) && !baseline) hr=win16_xaml_button_source(service,diagnostics,object,&source,&property_index);
    if (SUCCEEDED(hr) && !baseline && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    win16_xaml_button_style external={0};
    if (SUCCEEDED(hr) && !baseline) hr=win16_xaml_button_apply(service,diagnostics,object,&external);
    if (SUCCEEDED(hr) && !baseline) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr) && !baseline) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (external.button) { HRESULT removed=win16_xaml_button_remove(service,diagnostics,&external); if(SUCCEEDED(hr))hr=removed; }
    if (external.button) win16_xaml_button_discard(&external);
    if (combo) { (void)combo->vtable->put_open(combo,0); combo->vtable->release(combo); }
    if (popup) popup->vtable->release(popup);
    if (root_element) root_element->vtable->release(root_element);
    if (popup_object) IInspectable_Release(popup_object);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (selector) selector->vtable->release(selector);
    if (control) control->vtable->release(control);
    if (element) element->vtable->release(element);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (text) WindowsDeleteString(text);
    if (name) WindowsDeleteString(name);
    return hr;
}

static reader *combo_factory;
static IInspectable *combo_style;
static void CALLBACK test_combo_timer(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)window; (void)message; (void)time;
    KillTimer(NULL,timer);
    HRESULT candidate=test_combo_candidate(combo_factory,combo_style);
    IInspectable_Release(combo_style); combo_style=NULL;
    combo_factory->vtable->release(combo_factory); combo_factory=NULL;
    signal_named(SUCCEEDED(candidate) ? L"WIN16_XAML_COMBO_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
}

typedef struct test_range test_range;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_range *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_range *);
    ULONG (STDMETHODCALLTYPE *release)(test_range *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_range *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_range *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_range *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_minimum)(test_range *,double *);
    HRESULT (STDMETHODCALLTYPE *put_minimum)(test_range *,double);
    HRESULT (STDMETHODCALLTYPE *get_maximum)(test_range *,double *);
    HRESULT (STDMETHODCALLTYPE *put_maximum)(test_range *,double);
    void (STDMETHODCALLTYPE *unused[4])(void);
    HRESULT (STDMETHODCALLTYPE *get_value)(test_range *,double *);
    HRESULT (STDMETHODCALLTYPE *put_value)(test_range *,double);
} test_range_vtable;
struct test_range { const test_range_vtable *vtable; };
static const IID range_iid={0x540d6d61,0x8fac,0x5d5c,{0xb5,0xb0,0xe1,0x72,0xa7,0xdd,0xe1,0x03}};
typedef struct test_scroll test_scroll;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_scroll *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_scroll *);
    ULONG (STDMETHODCALLTYPE *release)(test_scroll *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_scroll *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_scroll *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_scroll *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_orientation)(test_scroll *,INT32 *);
    HRESULT (STDMETHODCALLTYPE *put_orientation)(test_scroll *,INT32);
    HRESULT (STDMETHODCALLTYPE *get_viewport)(test_scroll *,double *);
    HRESULT (STDMETHODCALLTYPE *put_viewport)(test_scroll *,double);
    HRESULT (STDMETHODCALLTYPE *get_indicator)(test_scroll *,INT32 *);
    HRESULT (STDMETHODCALLTYPE *put_indicator)(test_scroll *,INT32);
} test_scroll_vtable;
struct test_scroll { const test_scroll_vtable *vtable; };
static const IID scroll_iid={0x568cbf41,0xf741,0x5f05,{0x8e,0x08,0xc0,0xa5,0x0a,0xc1,0x7c,0x8c}};
typedef struct test_size test_size;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_size *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_size *);
    ULONG (STDMETHODCALLTYPE *release)(test_size *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_size *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_size *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_size *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[7])(void);
    HRESULT (STDMETHODCALLTYPE *get_actual_width)(test_size *,double *);
    HRESULT (STDMETHODCALLTYPE *get_actual_height)(test_size *,double *);
    HRESULT (STDMETHODCALLTYPE *get_width)(test_size *,double *);
    HRESULT (STDMETHODCALLTYPE *put_width)(test_size *,double);
    HRESULT (STDMETHODCALLTYPE *get_height)(test_size *,double *);
    HRESULT (STDMETHODCALLTYPE *put_height)(test_size *,double);
} test_size_vtable;
struct test_size { const test_size_vtable *vtable; };

static struct {
    test_window *window;
    test_scroll *scroll;
    test_size *thumb;
    test_ui *ui;
    double first;
    unsigned int phase;
} scroll_async;

static void CALLBACK test_scroll_async_timer(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)window; (void)message; (void)time;
    HRESULT hr=((test_layout *)scroll_async.ui)->vtable->update_layout((test_layout *)scroll_async.ui);
    double width=0;
    if (SUCCEEDED(hr)) hr=scroll_async.thumb->vtable->get_actual_width(scroll_async.thumb,&width);
    if (SUCCEEDED(hr) && !scroll_async.phase) {
        scroll_async.first=width;
        hr=scroll_async.scroll->vtable->put_viewport(scroll_async.scroll,80);
        if (SUCCEEDED(hr)) { scroll_async.phase=1; return; }
    } else if (SUCCEEDED(hr) && !(scroll_async.first>0 && width>scroll_async.first)) {
        WCHAR path[MAX_PATH]; DWORD length=GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_RESULT_FILE",path,ARRAYSIZE(path));
        if (length && length<ARRAYSIZE(path)) {
            HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,NULL);
            if (file!=INVALID_HANDLE_VALUE) {
                DWORD written=0; (void)WriteFile(file,&scroll_async.first,sizeof(double),&written,NULL);
                (void)WriteFile(file,&width,sizeof(double),&written,NULL); CloseHandle(file);
            }
        }
        hr=E_FAIL;
    }
    KillTimer(NULL,timer);
    signal_named(SUCCEEDED(hr) ? L"WIN16_XAML_SCROLL_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    (void)scroll_async.window->vtable->close(scroll_async.window);
    scroll_async.window->vtable->release(scroll_async.window);
    scroll_async.scroll->vtable->release(scroll_async.scroll);
    scroll_async.thumb->vtable->release(scroll_async.thumb);
    scroll_async.ui->vtable->release(scroll_async.ui);
    ZeroMemory(&scroll_async,sizeof(scroll_async));
}

static HRESULT test_scroll_arrow_visual(winui_element *root,const wchar_t *part)
{
    static const wchar_t vsm_name[]=L"Microsoft.UI.Xaml.VisualStateManager";
    HSTRING name=NULL; IInspectable *arrow=NULL; test_control *control=NULL; winui_vsm *manager=NULL;
    HRESULT hr=WindowsCreateString(part,(UINT32)lstrlenW(part),&name);
    if (SUCCEEDED(hr)) hr=root->vtable->find_name(root,name,&arrow);
    if (SUCCEEDED(hr) && !arrow) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(arrow,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(vsm_name,(UINT32)(ARRAYSIZE(vsm_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    static const wchar_t *states[]={L"Normal",L"Pressed",L"Disabled"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        HSTRING state=NULL; boolean changed=0;
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr) && i<2 && !check_edge_colors(arrow,i==1)) hr=E_FAIL;
        if (SUCCEEDED(hr) && i==2 && !check_disabled_text(arrow)) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    if (manager) manager->vtable->release(manager);
    if (control) control->vtable->release(control);
    if (arrow) IInspectable_Release(arrow);
    if (name) WindowsDeleteString(name);
    return hr;
}

static HRESULT test_scroll_candidate(IInspectable *style,INT32 orientation)
{
    static const wchar_t class_name[]=L"Microsoft.UI.Xaml.Controls.Primitives.ScrollBar";
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*thumb=NULL;
    BOOL keep_window=FALSE;
    BOOL baseline=GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_BASELINE",NULL,0)!=0;
    BOOL async=GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_ASYNC",NULL,0)!=0;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    unsigned int source=0,index=0;
    test_window *window=NULL; test_control *control=NULL; test_range *range=NULL; test_scroll *scroll=NULL;
    test_size *size=NULL,*thumb_size=NULL; winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL;
    test_ui *thumb_ui=NULL; INT32 thumb_visibility=-1;
    HRESULT hr=WindowsCreateString(class_name,(UINT32)(ARRAYSIZE(class_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&object);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_CONTENT_TEXT_MATCH_EVENT");
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&size);
    if (SUCCEEDED(hr)) hr=size->vtable->put_width(size,orientation ? 16 : 200);
    if (SUCCEEDED(hr)) hr=size->vtable->put_height(size,orientation ? 200 : 16);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&range_iid,(void **)&range);
    if (SUCCEEDED(hr)) hr=range->vtable->put_maximum(range,100);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&scroll_iid,(void **)&scroll);
    /* WinUI Orientation is Vertical=0, Horizontal=1. */
    if (SUCCEEDED(hr)) hr=scroll->vtable->put_orientation(scroll,orientation ? 0 : 1);
    if (SUCCEEDED(hr)) hr=scroll->vtable->put_viewport(scroll,20);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr) && !baseline && async) hr=element->vtable->put_style(element,style);
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr) && !baseline && !async) {
        info.Type=(BSTR)class_name; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr) && !baseline && !async) {
        hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if(hr==S_FALSE)hr=E_FAIL;
    }
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && !baseline && !async && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied && !GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_BASELINE",NULL,0)) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=scroll->vtable->put_indicator(scroll,2);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_STATES_EVENT");
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    const wchar_t *part=orientation ? L"VerticalThumb" : L"HorizontalThumb";
    if (SUCCEEDED(hr)) hr=WindowsCreateString(part,(UINT32)lstrlenW(part),&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&thumb);
    if (SUCCEEDED(hr) && !thumb) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_HANDLE_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(thumb,&winui_element_iid,(void **)&thumb_size);
    if (SUCCEEDED(hr) && GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_ASYNC",NULL,0)) {
        scroll_async.window=window; window->vtable->add_ref(window);
        scroll_async.scroll=scroll; scroll->vtable->add_ref(scroll);
        scroll_async.thumb=thumb_size; thumb_size->vtable->add_ref(thumb_size);
        scroll_async.ui=(test_ui *)ui; IInspectable_AddRef(ui);
        if (SetTimer(NULL,0,200,test_scroll_async_timer)) keep_window=TRUE;
        else {
            hr=HRESULT_FROM_WIN32(GetLastError());
            scroll_async.window->vtable->release(scroll_async.window);
            scroll_async.scroll->vtable->release(scroll_async.scroll);
            scroll_async.thumb->vtable->release(scroll_async.thumb);
            scroll_async.ui->vtable->release(scroll_async.ui);
            ZeroMemory(&scroll_async,sizeof(scroll_async));
        }
        goto cleanup;
    }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(thumb,&test_ui_iid,(void **)&thumb_ui);
    if (SUCCEEDED(hr)) hr=thumb_ui->vtable->get_visibility(thumb_ui,&thumb_visibility);
    if (SUCCEEDED(hr) && thumb_visibility==0) signal_named(L"WIN16_XAML_LOCAL_BINDING_EVENT");
    double first=0,second=0,value=-1;
    if (SUCCEEDED(hr)) hr=orientation ? thumb_size->vtable->get_actual_height(thumb_size,&first) : thumb_size->vtable->get_actual_width(thumb_size,&first);
    if (SUCCEEDED(hr) && first>0) signal_named(L"WIN16_XAML_LOCAL_BINDING_EVENT");
    if (SUCCEEDED(hr)) hr=scroll->vtable->put_viewport(scroll,80);
    if (SUCCEEDED(hr) && !GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_VIEWPORT_ONLY",NULL,0))
        hr=range->vtable->put_value(range,35);
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=orientation ? thumb_size->vtable->get_actual_height(thumb_size,&second) : thumb_size->vtable->get_actual_width(thumb_size,&second);
    if (SUCCEEDED(hr) && second>first) signal_named(L"WIN16_XAML_LOCAL_NULL_EVENT");
    if (SUCCEEDED(hr) && !(first>0 && second>first)) {
        WCHAR path[MAX_PATH]; DWORD length=GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_RESULT_FILE",path,ARRAYSIZE(path));
        if (length && length<ARRAYSIZE(path)) {
            HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,NULL);
            if (file!=INVALID_HANDLE_VALUE) {
                DWORD written=0; (void)WriteFile(file,&first,sizeof(first),&written,NULL);
                (void)WriteFile(file,&second,sizeof(second),&written,NULL);
                (void)WriteFile(file,&thumb_visibility,sizeof(thumb_visibility),&written,NULL); CloseHandle(file);
            }
        }
    }
    if (SUCCEEDED(hr) && !(first>0 && second>first)) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(orientation ? L"WIN16_XAML_LOCAL_NULL_EVENT" : L"WIN16_XAML_LOCAL_EVALUATED_EVENT");
    if (SUCCEEDED(hr)) hr=range->vtable->put_value(range,35);
    if (SUCCEEDED(hr)) hr=range->vtable->get_value(range,&value);
    if (SUCCEEDED(hr) && value!=35) hr=E_FAIL;
    if (SUCCEEDED(hr) && !baseline) hr=test_scroll_arrow_visual(root_element,orientation ? L"VerticalSmallIncrease" : L"HorizontalSmallIncrease");
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && !baseline && !async && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    win16_xaml_button_style external={0};
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_button_apply(service,diagnostics,object,&external);
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr) && !baseline && !async) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (external.button) { HRESULT removed=win16_xaml_button_remove(service,diagnostics,&external); if(SUCCEEDED(hr))hr=removed; }
    if (external.button) win16_xaml_button_discard(&external);
cleanup:
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (thumb_size) thumb_size->vtable->release(thumb_size);
    if (thumb_ui) thumb_ui->vtable->release(thumb_ui);
    if (root_element) root_element->vtable->release(root_element);
    if (thumb) IInspectable_Release(thumb);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (scroll) scroll->vtable->release(scroll);
    if (range) range->vtable->release(range);
    if (size) size->vtable->release(size);
    if (control) control->vtable->release(control);
    if (element) element->vtable->release(element);
    if (window) { if(!keep_window)(void)window->vtable->close(window); window->vtable->release(window); }
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_list test_list;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_list *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_list *);
    ULONG (STDMETHODCALLTYPE *release)(test_list *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_list *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_list *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_list *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[2])(void);
    HRESULT (STDMETHODCALLTYPE *put_mode)(test_list *,INT32);
    void (STDMETHODCALLTYPE *unused_select_all[3])(void);
    HRESULT (STDMETHODCALLTYPE *select_all)(test_list *);
} test_list_vtable;
struct test_list { const test_list_vtable *vtable; };
static const IID list_iid={0xf3e7dedf,0x7a3c,0x59f1,{0x9e,0x05,0xae,0x50,0x26,0xb5,0x42,0x93}};
typedef struct test_selector_item test_selector_item;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_selector_item *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_selector_item *);
    ULONG (STDMETHODCALLTYPE *release)(test_selector_item *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_selector_item *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_selector_item *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_selector_item *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_selected)(test_selector_item *,boolean *);
} test_selector_item_vtable;
struct test_selector_item { const test_selector_item_vtable *vtable; };
static const IID selector_item_iid={0x5772c4de,0x60ea,0x5492,{0x8c,0x5e,0xb3,0x32,0x3d,0x5a,0x3c,0xa6}};

static HRESULT test_list_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<ListBox xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml'><x:String>First</x:String><x:String>Second</x:String></ListBox>";
    HSTRING text=NULL,name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL;
    test_window *window=NULL; winui_element *element=NULL; test_control *control=NULL;
    test_selector *selector=NULL; test_item_mapping *mapping=NULL; test_list *list=NULL;
    test_selector_item *items[2]={NULL,NULL};
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    unsigned int source=0,property_index=0; winui_tree *tree=NULL;
    (void)style;
    HRESULT hr=WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&text);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,text,&object);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) { info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.ListBox"; relation.Parent=1; hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle); }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&property_index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&selector_iid,(void **)&selector);
    if (SUCCEEDED(hr)) hr=selector->vtable->put_index(selector,1);
    INT32 selected=-1;
    if (SUCCEEDED(hr)) hr=selector->vtable->get_index(selector,&selected);
    if (SUCCEEDED(hr) && selected!=1) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&item_mapping_iid,(void **)&mapping);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    for (INT32 i=0;SUCCEEDED(hr) && i<2;i++) {
        IInspectable *item=NULL; unsigned int source=0,index=0;
        hr=mapping->vtable->container_from_index(mapping,i,&item);
        if (SUCCEEDED(hr) && !item) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,item,&source,&index);
        if (SUCCEEDED(hr)) signal_named(source==BaseValueSourceDefault
            ? L"WIN16_XAML_SESSION_APPLIED_EVENT" : L"WIN16_XAML_SESSION_NOCHANGE_EVENT");
        VisualElement item_info={0}; ParentChildRelation item_relation={0};
        item_info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.ListBoxItem"; item_relation.Parent=1;
        if (SUCCEEDED(hr)) hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,item,&item_info.Handle);
        if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,item_relation,item_info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
        test_control *item_control=NULL; boolean item_applied=0;
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(item,&winui_control_iid,(void **)&item_control);
        if (SUCCEEDED(hr)) hr=item_control->vtable->apply_template(item_control,&item_applied);
        if (SUCCEEDED(hr) && !item_applied) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=test_combo_item_states(item,tree,item_control,i);
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(item,&selector_item_iid,(void **)&items[i]);
        if (item_control) item_control->vtable->release(item_control);
        if (item) IInspectable_Release(item);
    }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&list_iid,(void **)&list);
    if (SUCCEEDED(hr)) hr=list->vtable->put_mode(list,1);
    if (SUCCEEDED(hr)) hr=list->vtable->select_all(list);
    for (INT32 i=0;SUCCEEDED(hr) && i<2;i++) {
        boolean selected_item=0;
        hr=items[i]->vtable->get_selected(items[i],&selected_item);
        if (SUCCEEDED(hr) && !selected_item) hr=E_FAIL;
    }
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&property_index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    win16_xaml_button_style external={0};
    if (SUCCEEDED(hr)) hr=win16_xaml_button_apply(service,diagnostics,object,&external);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (external.button) { HRESULT removed=win16_xaml_button_remove(service,diagnostics,&external); if(SUCCEEDED(hr))hr=removed; }
    if (external.button) win16_xaml_button_discard(&external);
    if (tree) tree->vtable->release(tree);
    for (INT32 i=0;i<2;i++) if(items[i]) items[i]->vtable->release(items[i]);
    if (list) list->vtable->release(list);
    if (mapping) mapping->vtable->release(mapping);
    if (selector) selector->vtable->release(selector);
    if (control) control->vtable->release(control);
    if (element) element->vtable->release(element);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (text) WindowsDeleteString(text);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_progress test_progress;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_progress *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_progress *);
    ULONG (STDMETHODCALLTYPE *release)(test_progress *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_progress *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_progress *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_progress *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_indeterminate)(test_progress *,boolean *);
    HRESULT (STDMETHODCALLTYPE *put_indeterminate)(test_progress *,boolean);
    HRESULT (STDMETHODCALLTYPE *get_error)(test_progress *,boolean *);
    HRESULT (STDMETHODCALLTYPE *put_error)(test_progress *,boolean);
    HRESULT (STDMETHODCALLTYPE *get_paused)(test_progress *,boolean *);
    HRESULT (STDMETHODCALLTYPE *put_paused)(test_progress *,boolean);
} test_progress_vtable;
struct test_progress { const test_progress_vtable *vtable; };
static const IID progress_iid={0x87555c8c,0x0aaf,0x52c1,{0x83,0x90,0x0d,0xb1,0x7f,0x40,0x43,0x8e}};
typedef struct test_shape test_shape;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_shape *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_shape *);
    ULONG (STDMETHODCALLTYPE *release)(test_shape *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_shape *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_shape *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_shape *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_fill)(test_shape *,IInspectable **);
    void (STDMETHODCALLTYPE *put_fill)(void);
    HRESULT (STDMETHODCALLTYPE *get_stroke)(test_shape *,IInspectable **);
} test_shape_vtable;
struct test_shape { const test_shape_vtable *vtable; };
static const IID shape_iid={0x9941aad3,0x6af2,0x5ba2,{0x90,0x85,0x85,0x06,0xd5,0xf2,0x48,0x5e}};
static HRESULT test_shape_color(IInspectable *shape,BYTE r,BYTE g,BYTE b)
{
    test_shape *s=NULL; test_brush *brush=NULL; IInspectable *fill=NULL; test_color color={0};
    HRESULT hr=IInspectable_QueryInterface(shape,&shape_iid,(void **)&s);
    if (SUCCEEDED(hr)) hr=s->vtable->get_fill(s,&fill);
    if (SUCCEEDED(hr) && !fill) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(fill,&brush_iid,(void **)&brush);
    if (SUCCEEDED(hr)) hr=brush->vtable->get_color(brush,&color);
    if (SUCCEEDED(hr) && (color.r!=r || color.g!=g || color.b!=b)) hr=E_FAIL;
    if (brush) brush->vtable->release(brush);
    if (fill) IInspectable_Release(fill);
    if (s) s->vtable->release(s);
    return hr;
}
static HRESULT test_shape_stroke(IInspectable *shape,BYTE expected)
{
    test_shape *s=NULL; test_brush *brush=NULL; IInspectable *stroke=NULL; test_color color={0};
    HRESULT hr=IInspectable_QueryInterface(shape,&shape_iid,(void **)&s);
    if (SUCCEEDED(hr)) hr=s->vtable->get_stroke(s,&stroke);
    if (SUCCEEDED(hr) && !stroke) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(stroke,&brush_iid,(void **)&brush);
    if (SUCCEEDED(hr)) hr=brush->vtable->get_color(brush,&color);
    if (SUCCEEDED(hr) && (color.r!=expected || color.g!=expected || color.b!=expected)) hr=E_FAIL;
    if (brush) brush->vtable->release(brush);
    if (stroke) IInspectable_Release(stroke);
    if (s) s->vtable->release(s);
    return hr;
}
typedef struct test_transform test_transform;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_transform *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_transform *);
    ULONG (STDMETHODCALLTYPE *release)(test_transform *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_transform *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_transform *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_transform *,TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_x)(test_transform *,double *);
} test_transform_vtable;
struct test_transform { const test_transform_vtable *vtable; };
static const IID translate_iid={0xcfa21ca9,0xb79f,0x5450,{0xb4,0x59,0xa9,0x6c,0x48,0xcb,0x2c,0x8f}};
typedef struct test_composite test_composite;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_composite *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_composite *);
    ULONG (STDMETHODCALLTYPE *release)(test_composite *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_composite *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_composite *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_composite *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[14])(void);
    HRESULT (STDMETHODCALLTYPE *get_x)(test_composite *,double *);
} test_composite_vtable;
struct test_composite { const test_composite_vtable *vtable; };
static const IID composite_iid={0x55c5f8f3,0x20e4,0x5b80,{0xa0,0x46,0xce,0x4d,0x0f,0x62,0xf2,0xfe}};
typedef struct test_transform_owner test_transform_owner;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_transform_owner *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_transform_owner *);
    ULONG (STDMETHODCALLTYPE *release)(test_transform_owner *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(test_transform_owner *,ULONG *,IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(test_transform_owner *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(test_transform_owner *,TrustLevel *);
    void (STDMETHODCALLTYPE *unused[7])(void);
    HRESULT (STDMETHODCALLTYPE *get_transform)(test_transform_owner *,IInspectable **);
} test_transform_owner_vtable;
struct test_transform_owner { const test_transform_owner_vtable *vtable; };
static struct { test_window *window; test_transform *transform; double first; unsigned int phase; BOOL baseline; } progress_async;

static void CALLBACK test_progress_timer(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)window; (void)message; (void)time;
    double current=0; HRESULT hr=progress_async.baseline
        ? ((test_composite *)progress_async.transform)->vtable->get_x((test_composite *)progress_async.transform,&current)
        : progress_async.transform->vtable->get_x(progress_async.transform,&current);
    if (SUCCEEDED(hr) && !progress_async.phase) { progress_async.first=current; progress_async.phase=1; signal_named(L"WIN16_XAML_SESSION_APPLIED_EVENT"); return; }
    if (SUCCEEDED(hr) && current==progress_async.first) hr=E_FAIL;
    KillTimer(NULL,timer);
    signal_named(SUCCEEDED(hr) ? L"WIN16_XAML_PROGRESS_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    (void)progress_async.window->vtable->close(progress_async.window);
    progress_async.window->vtable->release(progress_async.window);
    if (progress_async.transform) progress_async.transform->vtable->release(progress_async.transform);
    ZeroMemory(&progress_async,sizeof(progress_async));
}

static HRESULT test_progress_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t class_name[]=L"Microsoft.UI.Xaml.Controls.ProgressBar";
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*indicator=NULL,*indeterminate=NULL;
    BOOL keep_window=FALSE;
    BOOL baseline=GetEnvironmentVariableW(L"WIN16_XAML_PROGRESS_BASELINE",NULL,0)!=0;
    BOOL async=GetEnvironmentVariableW(L"WIN16_XAML_PROGRESS_ASYNC",NULL,0)!=0;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    unsigned int source=0,index=0;
    test_window *window=NULL; test_control *control=NULL; test_range *range=NULL; test_progress *progress=NULL;
    test_size *size=NULL,*indicator_size=NULL; test_ui *indeterminate_ui=NULL;
    winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL;
    (void)class_name;
    static const wchar_t markup[]=L"<ProgressBar xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation'/>";
    HSTRING markup_text=NULL;
    HRESULT hr=WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&markup_text);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,markup_text,&object);
    if (markup_text) WindowsDeleteString(markup_text);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_CONTENT_TEXT_MATCH_EVENT");
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&size);
    if (SUCCEEDED(hr)) hr=size->vtable->put_width(size,200);
    if (SUCCEEDED(hr)) hr=size->vtable->put_height(size,20);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&range_iid,(void **)&range);
    if (SUCCEEDED(hr)) hr=range->vtable->put_maximum(range,100);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr) && !baseline && async) hr=element->vtable->put_style(element,style);
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr) && !baseline && !async) {
        info.Type=(BSTR)class_name; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr) && !baseline && !async) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && !baseline && !async && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied && !baseline) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_STATES_EVENT");
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"DeterminateProgressBarIndicator",31,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&indicator);
    if (SUCCEEDED(hr) && !indicator) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"IndeterminateProgressBarIndicator",33,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&indeterminate);
    if (SUCCEEDED(hr) && !indeterminate) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_HANDLE_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(indicator,&winui_element_iid,(void **)&indicator_size);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(indeterminate,&test_ui_iid,(void **)&indeterminate_ui);
    if (SUCCEEDED(hr)) hr=range->vtable->put_value(range,25);
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    double first=0,second=0,opacity=0;
    if (SUCCEEDED(hr)) hr=indicator_size->vtable->get_width(indicator_size,&first);
    if (SUCCEEDED(hr)) hr=range->vtable->put_value(range,75);
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=indicator_size->vtable->get_width(indicator_size,&second);
    if (SUCCEEDED(hr) && !(first>0 && second>first)) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_BINDING_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&progress_iid,(void **)&progress);
    if (SUCCEEDED(hr) && !baseline && !async) hr=progress->vtable->put_paused(progress,1);
    if (SUCCEEDED(hr) && !baseline && !async) hr=test_shape_color(indicator,128,128,128);
    if (SUCCEEDED(hr) && !baseline && !async) hr=progress->vtable->put_paused(progress,0);
    if (SUCCEEDED(hr) && !baseline && !async) hr=progress->vtable->put_error(progress,1);
    if (SUCCEEDED(hr) && !baseline && !async) hr=test_shape_color(indicator,128,0,0);
    if (SUCCEEDED(hr) && !baseline && !async) hr=progress->vtable->put_error(progress,0);
    if (SUCCEEDED(hr)) hr=progress->vtable->put_indeterminate(progress,1);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_EVALUATED_EVENT");
    if (SUCCEEDED(hr)) hr=indeterminate_ui->vtable->get_opacity(indeterminate_ui,&opacity);
    if (SUCCEEDED(hr) && opacity!=1) hr=E_FAIL;
    BOOL transitions=!async || GetEnvironmentVariableW(L"WIN16_XAML_PROGRESS_TRANSITIONS",NULL,0)!=0;
    if (SUCCEEDED(hr) && transitions) hr=progress->vtable->put_paused(progress,1);
    if (SUCCEEDED(hr) && !baseline && transitions) hr=test_shape_color(indeterminate,128,128,128);
    if (SUCCEEDED(hr) && transitions) hr=progress->vtable->put_paused(progress,0);
    if (SUCCEEDED(hr) && transitions) hr=progress->vtable->put_error(progress,1);
    if (SUCCEEDED(hr) && !baseline && transitions) hr=test_shape_color(indeterminate,128,0,0);
    if (SUCCEEDED(hr) && transitions) hr=progress->vtable->put_error(progress,0);
    if (SUCCEEDED(hr) && GetEnvironmentVariableW(L"WIN16_XAML_PROGRESS_ASYNC",NULL,0)) {
        test_transform_owner *owner=NULL; IInspectable *transform_object=NULL; test_transform *transform=NULL;
        hr=IInspectable_QueryInterface(indeterminate,&test_ui_iid,(void **)&owner);
        if (SUCCEEDED(hr)) hr=owner->vtable->get_transform(owner,&transform_object);
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(transform_object,baseline ? &composite_iid : &translate_iid,(void **)&transform);
        if (SUCCEEDED(hr)) {
            progress_async.window=window; window->vtable->add_ref(window);
            progress_async.transform=transform; if(transform)transform->vtable->add_ref(transform);
            progress_async.baseline=baseline;
            if (SetTimer(NULL,0,300,test_progress_timer)) { keep_window=TRUE; signal_named(L"WIN16_XAML_LOCAL_NULL_EVENT"); }
            else {
                hr=HRESULT_FROM_WIN32(GetLastError());
                progress_async.window->vtable->release(progress_async.window);
                if(progress_async.transform)progress_async.transform->vtable->release(progress_async.transform);
                ZeroMemory(&progress_async,sizeof(progress_async));
            }
        }
        if (transform) transform->vtable->release(transform);
        if (transform_object) IInspectable_Release(transform_object);
        if (owner) owner->vtable->release(owner);
    }
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && !baseline && !async && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    win16_xaml_button_style external={0};
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_button_apply(service,diagnostics,object,&external);
    if (SUCCEEDED(hr) && !baseline && !async) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr) && !baseline && !async) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (adapter.service) { HRESULT stopped=win16_xaml_adapter_stop(&adapter); if(SUCCEEDED(hr))hr=stopped; }
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (external.button) { HRESULT removed=win16_xaml_button_remove(service,diagnostics,&external); if(SUCCEEDED(hr))hr=removed; }
    if (external.button) win16_xaml_button_discard(&external);
    if (progress) progress->vtable->release(progress);
    if (indeterminate_ui) indeterminate_ui->vtable->release(indeterminate_ui);
    if (indicator_size) indicator_size->vtable->release(indicator_size);
    if (root_element) root_element->vtable->release(root_element);
    if (indeterminate) IInspectable_Release(indeterminate);
    if (indicator) IInspectable_Release(indicator);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (range) range->vtable->release(range);
    if (size) size->vtable->release(size);
    if (control) control->vtable->release(control);
    if (element) element->vtable->release(element);
    if (window) { if(!keep_window)(void)window->vtable->close(window); window->vtable->release(window); }
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

static reader *progress_factory;
static IInspectable *progress_style;
static void CALLBACK test_progress_start_timer(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)window; (void)message; (void)time;
    KillTimer(NULL,timer);
    HRESULT result=test_progress_candidate(progress_factory,progress_style);
    IInspectable_Release(progress_style); progress_style=NULL;
    progress_factory->vtable->release(progress_factory); progress_factory=NULL;
    if (!GetEnvironmentVariableW(L"WIN16_XAML_PROGRESS_ASYNC",NULL,0) || FAILED(result))
        signal_named(SUCCEEDED(result) ? L"WIN16_XAML_PROGRESS_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
}

typedef struct test_switch test_switch;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_switch *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_switch *);
    ULONG (STDMETHODCALLTYPE *release)(test_switch *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_on)(test_switch *,boolean *);
    HRESULT (STDMETHODCALLTYPE *put_on)(test_switch *,boolean);
} test_switch_vtable;
struct test_switch { const test_switch_vtable *vtable; };
static const IID test_switch_iid={0x1b17eeb1,0x74bf,0x5a83,{0x81,0x61,0xa8,0x6f,0x0f,0xdc,0xdf,0x24}};

static HRESULT test_switch_content(IInspectable *object,const wchar_t *expected)
{
    content_interface *presenter=NULL; IInspectable *content=NULL;
    __x_ABI_CWindows_CFoundation_CIPropertyValue *value=NULL; HSTRING text=NULL;
    HRESULT hr=IInspectable_QueryInterface(object,&winui_presenter_iid,(void **)&presenter);
    if (SUCCEEDED(hr)) hr=presenter->vtable->get_content(presenter,&content);
    if (SUCCEEDED(hr) && !content) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(content,&property_value_iid,(void **)&value);
    if (SUCCEEDED(hr)) hr=__x_ABI_CWindows_CFoundation_CIPropertyValue_GetString(value,&text);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(text,NULL),expected)) hr=E_FAIL;
    if (text) WindowsDeleteString(text);
    if (value) __x_ABI_CWindows_CFoundation_CIPropertyValue_Release(value);
    if (content) IInspectable_Release(content);
    if (presenter) presenter->vtable->release(presenter);
    return hr;
}

static HRESULT test_switch_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<ToggleSwitch xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' Header='Header' OnContent='On' OffContent='Off'/>";
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*mark=NULL;
    IInspectable *on_content=NULL,*off_content=NULL,*header=NULL,*focus=NULL; test_ui *on_ui=NULL,*off_ui=NULL,*focus_ui=NULL;
    winui_vsm *manager=NULL;
    winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL;
    test_window *window=NULL; test_control *control=NULL; test_switch *toggle=NULL; test_ui *mark_ui=NULL;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    (void)style;
    HRESULT hr=WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.ToggleSwitch"; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Mark",4,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&mark);
    if (SUCCEEDED(hr) && !mark) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(mark,&test_ui_iid,(void **)&mark_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"OnContentPresenter",18,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&on_content);
    if (SUCCEEDED(hr) && !on_content) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(on_content,&test_ui_iid,(void **)&on_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"OffContentPresenter",19,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&off_content);
    if (SUCCEEDED(hr) && !off_content) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(off_content,&test_ui_iid,(void **)&off_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"HeaderContentPresenter",22,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&header);
    if (SUCCEEDED(hr) && !header) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=test_switch_content(header,L"Header");
    if (SUCCEEDED(hr)) hr=test_switch_content(on_content,L"On");
    if (SUCCEEDED(hr)) hr=test_switch_content(off_content,L"Off");
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"FocusFrame",10,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&focus);
    if (SUCCEEDED(hr) && !focus) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(focus,&test_ui_iid,(void **)&focus_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.VisualStateManager",36,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    for (unsigned int i=0;SUCCEEDED(hr) && i<2;i++) {
        const wchar_t *state=i ? L"Unfocused" : L"Focused"; HSTRING state_name=NULL;
        boolean changed=0; double opacity=-1;
        hr=WindowsCreateString(state,(UINT32)lstrlenW(state),&state_name);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state_name,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=focus_ui->vtable->get_opacity(focus_ui,&opacity);
        if (SUCCEEDED(hr) && opacity!=(i ? 0.0 : 1.0)) hr=E_FAIL;
        if (state_name) WindowsDeleteString(state_name);
    }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_switch_iid,(void **)&toggle);
    for (unsigned int i=0;SUCCEEDED(hr) && i<3;i++) {
        boolean value=(boolean)(i==1); double opacity=-1,on=-1,off=-1;
        hr=toggle->vtable->put_on(toggle,value);
        if (SUCCEEDED(hr)) hr=mark_ui->vtable->get_opacity(mark_ui,&opacity);
        if (SUCCEEDED(hr) && opacity!=(double)value) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=on_ui->vtable->get_opacity(on_ui,&on);
        if (SUCCEEDED(hr)) hr=off_ui->vtable->get_opacity(off_ui,&off);
        if (SUCCEEDED(hr) && (on!=(double)value || off!=(double)!value)) hr=E_FAIL;
    }
    unsigned int source=0,index=0;
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (off_ui) off_ui->vtable->release(off_ui);
    if (on_ui) on_ui->vtable->release(on_ui);
    if (manager) manager->vtable->release(manager);
    if (focus_ui) focus_ui->vtable->release(focus_ui);
    if (focus) IInspectable_Release(focus);
    if (header) IInspectable_Release(header);
    if (off_content) IInspectable_Release(off_content);
    if (on_content) IInspectable_Release(on_content);
    if (toggle) toggle->vtable->release(toggle);
    if (mark_ui) mark_ui->vtable->release(mark_ui);
    if (mark) IInspectable_Release(mark);
    if (root_element) root_element->vtable->release(root_element);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (control) control->vtable->release(control);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) element->vtable->release(element);
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_offset test_offset;
typedef struct { float x,y,z; } test_vector3;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_offset *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_offset *);
    ULONG (STDMETHODCALLTYPE *release)(test_offset *);
    void (STDMETHODCALLTYPE *unused[104])(void);
    HRESULT (STDMETHODCALLTYPE *get_offset)(test_offset *,test_vector3 *);
} test_offset_vtable;
struct test_offset { const test_offset_vtable *vtable; };

typedef struct test_slider test_slider;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_slider *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_slider *);
    ULONG (STDMETHODCALLTYPE *release)(test_slider *);
    void (STDMETHODCALLTYPE *unused[16])(void);
    HRESULT (STDMETHODCALLTYPE *put_reversed)(test_slider *,boolean);
} test_slider_vtable;
struct test_slider { const test_slider_vtable *vtable; };
static const IID test_slider_iid={0xf7418ecf,0x7c35,0x5216,{0x8b,0xf1,0xd8,0x2d,0x47,0xcc,0xe5,0xdf}};

static HRESULT test_slider_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<Slider xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' TickFrequency='10' TickPlacement='BottomRight'/>";
    static const wchar_t vertical_markup[]=L"<Slider xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' Orientation='Vertical' TickFrequency='10' TickPlacement='BottomRight'/>";
    BOOL vertical=GetEnvironmentVariableW(L"WIN16_XAML_SLIDER_VERTICAL",NULL,0)!=0;
    BOOL reversed=GetEnvironmentVariableW(L"WIN16_XAML_SLIDER_REVERSED",NULL,0)!=0;
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*decrease=NULL,*thumb=NULL,*ticks=NULL,*focus=NULL;
    winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL;
    test_window *window=NULL; test_control *control=NULL; test_range *range=NULL; test_size *size=NULL,*decrease_size=NULL;
    test_offset *thumb_offset=NULL; test_ui *ticks_ui=NULL,*thumb_ui=NULL,*focus_ui=NULL; winui_vsm *manager=NULL; test_slider *slider=NULL;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    (void)style;
    const wchar_t *chosen=vertical ? vertical_markup : markup;
    HRESULT hr=WindowsCreateString(chosen,(UINT32)lstrlenW(chosen),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&size);
    if (SUCCEEDED(hr)) hr=size->vtable->put_width(size,vertical ? 30 : 200);
    if (SUCCEEDED(hr)) hr=size->vtable->put_height(size,vertical ? 200 : 30);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&range_iid,(void **)&range);
    if (SUCCEEDED(hr)) hr=range->vtable->put_maximum(range,100);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_slider_iid,(void **)&slider);
    if (SUCCEEDED(hr) && reversed) hr=slider->vtable->put_reversed(slider,1);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.Slider"; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    const wchar_t *part=vertical ? L"VerticalDecreaseRect" : L"HorizontalDecreaseRect";
    if (SUCCEEDED(hr)) hr=WindowsCreateString(part,(UINT32)lstrlenW(part),&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&decrease);
    if (SUCCEEDED(hr) && !decrease) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(decrease,&winui_element_iid,(void **)&decrease_size);
    if (name) { WindowsDeleteString(name); name=NULL; }
    part=vertical ? L"VerticalThumb" : L"HorizontalThumb";
    if (SUCCEEDED(hr)) hr=WindowsCreateString(part,(UINT32)lstrlenW(part),&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&thumb);
    if (SUCCEEDED(hr) && !thumb) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(thumb,&test_ui_iid,(void **)&thumb_offset);
    if (name) { WindowsDeleteString(name); name=NULL; }
    part=vertical ? L"RightTickBar" : L"BottomTickBar";
    if (SUCCEEDED(hr)) hr=WindowsCreateString(part,(UINT32)lstrlenW(part),&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&ticks);
    if (SUCCEEDED(hr) && !ticks) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(ticks,&test_ui_iid,(void **)&ticks_ui);
    INT32 visibility=-1;
    if (SUCCEEDED(hr)) hr=ticks_ui->vtable->get_visibility(ticks_ui,&visibility);
    if (SUCCEEDED(hr) && visibility!=0) hr=E_FAIL;
    double first=0,second=0,value=0;
    test_vector3 first_offset={0},second_offset={0};
    if (SUCCEEDED(hr)) hr=range->vtable->put_value(range,25);
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=vertical ? decrease_size->vtable->get_actual_height(decrease_size,&first) : decrease_size->vtable->get_actual_width(decrease_size,&first);
    if (SUCCEEDED(hr)) hr=thumb_offset->vtable->get_offset(thumb_offset,&first_offset);
    if (SUCCEEDED(hr)) hr=range->vtable->put_value(range,75);
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=vertical ? decrease_size->vtable->get_actual_height(decrease_size,&second) : decrease_size->vtable->get_actual_width(decrease_size,&second);
    if (SUCCEEDED(hr)) hr=thumb_offset->vtable->get_offset(thumb_offset,&second_offset);
    if (SUCCEEDED(hr)) hr=range->vtable->get_value(range,&value);
    if (SUCCEEDED(hr) && !(second>first && value==75)) hr=E_FAIL;
    if (SUCCEEDED(hr) && !(vertical ? (reversed ? second_offset.y>first_offset.y : second_offset.y<first_offset.y)
                                     : (reversed ? second_offset.x<first_offset.x : second_offset.x>first_offset.x))) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(thumb,&test_ui_iid,(void **)&thumb_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"FocusFrame",10,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&focus);
    if (SUCCEEDED(hr) && !focus) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(focus,&test_ui_iid,(void **)&focus_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.VisualStateManager",36,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    static const wchar_t *states[]={L"Disabled",L"Normal",L"Focused",L"Unfocused"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        HSTRING state=NULL; boolean changed=0; double opacity=-1;
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=(i<2 ? thumb_ui : focus_ui)->vtable->get_opacity(i<2 ? thumb_ui : focus_ui,&opacity);
        if (SUCCEEDED(hr) && opacity!=(i==0 ? 0.5 : i==3 ? 0.0 : 1.0)) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    unsigned int source=0,index=0;
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (manager) manager->vtable->release(manager);
    if (focus_ui) focus_ui->vtable->release(focus_ui);
    if (thumb_ui) thumb_ui->vtable->release(thumb_ui);
    if (focus) IInspectable_Release(focus);
    if (ticks_ui) ticks_ui->vtable->release(ticks_ui);
    if (ticks) IInspectable_Release(ticks);
    if (thumb_offset) thumb_offset->vtable->release(thumb_offset);
    if (thumb) IInspectable_Release(thumb);
    if (decrease_size) decrease_size->vtable->release(decrease_size);
    if (decrease) IInspectable_Release(decrease);
    if (root_element) root_element->vtable->release(root_element);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (control) control->vtable->release(control);
    if (slider) slider->vtable->release(slider);
    if (range) range->vtable->release(range);
    if (size) size->vtable->release(size);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) element->vtable->release(element);
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_password test_password;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_password *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_password *);
    ULONG (STDMETHODCALLTYPE *release)(test_password *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_password)(test_password *,HSTRING *);
    HRESULT (STDMETHODCALLTYPE *put_password)(test_password *,HSTRING);
    void (STDMETHODCALLTYPE *unused[16])(void);
    HRESULT (STDMETHODCALLTYPE *get_reveal_mode)(test_password *,INT32 *);
    HRESULT (STDMETHODCALLTYPE *put_reveal_mode)(test_password *,INT32);
} test_password_vtable;
struct test_password { const test_password_vtable *vtable; };
static const IID test_password_iid={0x6d3ccff7,0xaaee,0x5adc,{0x82,0x98,0x33,0x30,0x0f,0xa1,0x19,0xda}};

static HRESULT test_password_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<PasswordBox xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' PlaceholderText='Test hint'/>";
    HSTRING name=NULL,fake=NULL,actual=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*placeholder=NULL,*reveal=NULL;
    winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL; winui_vsm *manager=NULL;
    test_window *window=NULL; test_control *control=NULL; test_password *password=NULL; test_ui *placeholder_ui=NULL,*reveal_ui=NULL;
    HRESULT hr=WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=element->vtable->put_style(element,style);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"PlaceholderTextContentPresenter",31,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&placeholder);
    if (SUCCEEDED(hr) && !placeholder) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(placeholder,&test_ui_iid,(void **)&placeholder_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"RevealButton",12,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&reveal);
    if (SUCCEEDED(hr) && !reveal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(reveal,&test_ui_iid,(void **)&reveal_ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_password_iid,(void **)&password);
    for (INT32 i=0;SUCCEEDED(hr) && i<3;i++) {
        INT32 mode=-1;
        hr=password->vtable->put_reveal_mode(password,i);
        if (SUCCEEDED(hr)) hr=password->vtable->get_reveal_mode(password,&mode);
        if (SUCCEEDED(hr) && mode!=i) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=password->vtable->put_reveal_mode(password,0);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"dummy-test",10,&fake);
    if (SUCCEEDED(hr)) hr=password->vtable->put_password(password,fake);
    if (SUCCEEDED(hr)) hr=password->vtable->get_password(password,&actual);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(actual,NULL),L"dummy-test")) hr=E_FAIL;
    if (SUCCEEDED(hr)) {
        HRESULT bitmap_result=E_FAIL;
        int bitmap=check_rendered_pixels(object,&bitmap_result);
        signal_named(bitmap==1 ? L"WIN16_XAML_BITMAP_OK_EVENT" : L"WIN16_XAML_BITMAP_FAILED_EVENT");
        if (bitmap!=1) {
            WCHAR variable[64];
            if (bitmap_result==E_NOTIMPL) signal_named(L"WIN16_XAML_BITMAP_NOTIMPL_EVENT");
            else if (bitmap_result==E_ACCESSDENIED) signal_named(L"WIN16_XAML_BITMAP_DENIED_EVENT");
            else if (bitmap_result==E_INVALIDARG) signal_named(L"WIN16_XAML_BITMAP_INVALID_EVENT");
            else if (bitmap_result==E_FAIL) signal_named(L"WIN16_XAML_BITMAP_EFAIL_EVENT");
            else signal_named(L"WIN16_XAML_BITMAP_OTHER_EVENT");
            swprintf(variable,ARRAYSIZE(variable),L"WIN16_XAML_BITMAP_STAGE_%d_EVENT",bitmap);
            signal_named(variable);
        }
    }
    INT32 visibility=-1;
    if (SUCCEEDED(hr)) hr=placeholder_ui->vtable->get_visibility(placeholder_ui,&visibility);
    if (SUCCEEDED(hr) && visibility!=1) hr=E_FAIL;
    HSTRING empty=NULL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"",0,&empty);
    if (SUCCEEDED(hr)) hr=password->vtable->put_password(password,empty);
    if (SUCCEEDED(hr)) hr=placeholder_ui->vtable->get_visibility(placeholder_ui,&visibility);
    if (SUCCEEDED(hr) && visibility!=0) hr=E_FAIL;
    if (empty) WindowsDeleteString(empty);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.VisualStateManager",36,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    static const wchar_t *states[]={L"ButtonVisible",L"ButtonCollapsed"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<2;i++) {
        HSTRING state=NULL; boolean changed=0; visibility=-1;
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=reveal_ui->vtable->get_visibility(reveal_ui,&visibility);
        if (SUCCEEDED(hr) && visibility!=(INT32)i) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    for (INT32 mode=1;SUCCEEDED(hr) && mode<=2;mode++) {
        HSTRING state=NULL; boolean changed=0; visibility=-1;
        hr=password->vtable->put_reveal_mode(password,0);
        if (SUCCEEDED(hr)) hr=WindowsCreateString(L"ButtonVisible",13,&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=password->vtable->put_reveal_mode(password,mode);
        if (SUCCEEDED(hr)) hr=reveal_ui->vtable->get_visibility(reveal_ui,&visibility);
        if (SUCCEEDED(hr) && visibility!=1) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    if (SUCCEEDED(hr)) hr=password->vtable->put_reveal_mode(password,0);
    test_control *reveal_control=NULL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(reveal,&winui_control_iid,(void **)&reveal_control);
    if (SUCCEEDED(hr)) hr=reveal_control->vtable->apply_template(reveal_control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    static const wchar_t *button_states[]={L"Normal",L"Pressed"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<2;i++) {
        HSTRING state=NULL; boolean changed=0;
        hr=WindowsCreateString(button_states[i],(UINT32)lstrlenW(button_states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)reveal_control,state,0,&changed);
        if (SUCCEEDED(hr) && (!changed || !check_edge_colors(reveal,i==1))) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    if (reveal_control) reveal_control->vtable->release(reveal_control);
    if (manager) manager->vtable->release(manager);
    if (password) password->vtable->release(password);
    if (reveal_ui) reveal_ui->vtable->release(reveal_ui);
    if (placeholder_ui) placeholder_ui->vtable->release(placeholder_ui);
    if (reveal) IInspectable_Release(reveal);
    if (placeholder) IInspectable_Release(placeholder);
    if (root_element) root_element->vtable->release(root_element);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (control) control->vtable->release(control);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) element->vtable->release(element);
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (actual) WindowsDeleteString(actual);
    if (fake) WindowsDeleteString(fake);
    if (name) WindowsDeleteString(name);
    return hr;
}

static HRESULT test_toggle_button_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<ToggleButton xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' Content='Test'/>";
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL;
    winui_element *element=NULL; test_window *window=NULL; test_control *control=NULL; test_toggle *toggle=NULL;
    __x_ABI_CWindows_CFoundation_CIPropertyValueStatics *properties=NULL;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    (void)style;
    HRESULT hr=WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.Primitives.ToggleButton"; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_STATES_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_toggle_iid,(void **)&toggle);
    if (SUCCEEDED(hr)) hr=toggle->vtable->put_three(toggle,1);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Windows.Foundation.PropertyValue",32,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics,(void **)&properties);
    for (unsigned int i=0;SUCCEEDED(hr) && i<3;i++) {
        IInspectable *boxed=NULL; IUnknown *reference=NULL;
        if (i<2) hr=__x_ABI_CWindows_CFoundation_CIPropertyValueStatics_CreateBoolean(properties,(boolean)i,&boxed);
        if (SUCCEEDED(hr) && boxed) hr=IInspectable_QueryInterface(boxed,&IID___FIReference_1_boolean,(void **)&reference);
        if (SUCCEEDED(hr)) hr=toggle->vtable->put_checked(toggle,reference);
        if (SUCCEEDED(hr) && !check_edge_colors(object,i==1)) hr=E_FAIL;
        if (SUCCEEDED(hr)) signal_named(i==0 ? L"WIN16_XAML_LOCAL_BINDING_EVENT" : i==1 ? L"WIN16_XAML_LOCAL_EVALUATED_EVENT" : L"WIN16_XAML_LOCAL_NULL_EVENT");
        if (reference) IUnknown_Release(reference);
        if (boxed) IInspectable_Release(boxed);
    }
    unsigned int source=0,index=0;
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (properties) __x_ABI_CWindows_CFoundation_CIPropertyValueStatics_Release(properties);
    if (toggle) toggle->vtable->release(toggle);
    if (control) control->vtable->release(control);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) element->vtable->release(element);
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_menu_item test_menu_item;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_menu_item *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_menu_item *);
    ULONG (STDMETHODCALLTYPE *release)(test_menu_item *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_text)(test_menu_item *,HSTRING *);
} test_menu_item_vtable;
struct test_menu_item { const test_menu_item_vtable *vtable; };
static const IID test_menu_item_iid={0x4252df5a,0x44f9,0x5ee8,{0xb1,0xcc,0x53,0xde,0x9a,0xaa,0x40,0x95}};

typedef struct test_button_flyout test_button_flyout;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_button_flyout *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_button_flyout *);
    ULONG (STDMETHODCALLTYPE *release)(test_button_flyout *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_flyout)(test_button_flyout *,IInspectable **);
} test_button_flyout_vtable;
struct test_button_flyout { const test_button_flyout_vtable *vtable; };
static const IID test_button_flyout_iid={0x216c183d,0xd07a,0x5aa5,{0xb8,0xa4,0x03,0x00,0xa2,0x68,0x3e,0x87}};
typedef struct test_flyout test_flyout;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_flyout *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_flyout *);
    ULONG (STDMETHODCALLTYPE *release)(test_flyout *);
    void (STDMETHODCALLTYPE *unused_to_open[24])(void);
    HRESULT (STDMETHODCALLTYPE *get_open)(test_flyout *,boolean *);
    HRESULT (STDMETHODCALLTYPE *get_xaml_root)(test_flyout *,IInspectable **);
    void (STDMETHODCALLTYPE *unused_to_show[9])(void);
    HRESULT (STDMETHODCALLTYPE *show_at)(test_flyout *,IInspectable *);
    void (STDMETHODCALLTYPE *show_at_options)(void);
    HRESULT (STDMETHODCALLTYPE *hide)(test_flyout *);
} test_flyout_vtable;
struct test_flyout { const test_flyout_vtable *vtable; };
static const IID test_flyout_iid={0xbb6603bf,0x744d,0x5c31,{0xa8,0x7d,0x74,0x43,0x94,0x63,0x4d,0x77}};

typedef struct test_popup_tree test_popup_tree;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_popup_tree *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_popup_tree *);
    ULONG (STDMETHODCALLTYPE *release)(test_popup_tree *);
    void (STDMETHODCALLTYPE *unused[12])(void);
    HRESULT (STDMETHODCALLTYPE *get_popups)(test_popup_tree *,IInspectable *,IInspectable **);
} test_popup_tree_vtable;
struct test_popup_tree { const test_popup_tree_vtable *vtable; };
typedef struct test_popup_list test_popup_list;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_popup_list *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_popup_list *);
    ULONG (STDMETHODCALLTYPE *release)(test_popup_list *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_at)(test_popup_list *,UINT32,IInspectable **);
    HRESULT (STDMETHODCALLTYPE *get_size)(test_popup_list *,UINT32 *);
} test_popup_list_vtable;
struct test_popup_list { const test_popup_list_vtable *vtable; };
typedef struct test_popup_child test_popup_child;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_popup_child *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_popup_child *);
    ULONG (STDMETHODCALLTYPE *release)(test_popup_child *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_child)(test_popup_child *,IInspectable **);
} test_popup_child_vtable;
struct test_popup_child { const test_popup_child_vtable *vtable; };

static BOOL find_menu_presenter(IInspectable *object,unsigned int depth,void *context)
{
    (void)depth; IInspectable **found=context; HSTRING runtime=NULL;
    if (SUCCEEDED(IInspectable_GetRuntimeClassName(object,&runtime)) && runtime &&
        !wcscmp(WindowsGetStringRawBuffer(runtime,NULL),L"Microsoft.UI.Xaml.Controls.MenuFlyoutPresenter")) {
        *found=object; IInspectable_AddRef(object);
    }
    if (runtime) WindowsDeleteString(runtime);
    return !*found;
}

static HRESULT test_menu_popup(reader *factory)
{
    static const wchar_t markup[]=L"<Button xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' Content='Test'><Button.Flyout><MenuFlyout><MenuFlyoutItem Text='First'/><MenuFlyoutItem Text='Second'/></MenuFlyout></Button.Flyout></Button>";
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*flyout_object=NULL;
    test_window *window=NULL; test_button_flyout *button=NULL; test_flyout *flyout=NULL; winui_element *target=NULL;
    HRESULT hr=WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&target);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_STATES_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_button_flyout_iid,(void **)&button);
    if (SUCCEEDED(hr)) hr=button->vtable->get_flyout(button,&flyout_object);
    if (SUCCEEDED(hr) && !flyout_object) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(flyout_object,&test_flyout_iid,(void **)&flyout);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_BINDING_EVENT");
    if (SUCCEEDED(hr)) hr=flyout->vtable->show_at(flyout,(IInspectable *)target);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_EVALUATED_EVENT");
    boolean open=0;
    if (SUCCEEDED(hr)) hr=flyout->vtable->get_open(flyout,&open);
    if (SUCCEEDED(hr) && !open) hr=E_FAIL;
    IInspectable *xaml_root=NULL,*popup_list=NULL,*popup_object=NULL,*child=NULL,*presenter=NULL;
    test_popup_tree *popup_tree=NULL; test_popup_child *popup=NULL; win16_xaml_adapter adapter={0};
    if (SUCCEEDED(hr)) hr=flyout->vtable->get_xaml_root(flyout,&xaml_root);
    if (SUCCEEDED(hr) && !xaml_root) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&popup_tree);
    if (SUCCEEDED(hr)) hr=popup_tree->vtable->get_popups(popup_tree,xaml_root,&popup_list);
    UINT32 count=0;
    if (SUCCEEDED(hr)) hr=((test_popup_list *)popup_list)->vtable->get_size((test_popup_list *)popup_list,&count);
    for (UINT32 i=0;SUCCEEDED(hr) && i<count && !presenter;i++) {
        hr=((test_popup_list *)popup_list)->vtable->get_at((test_popup_list *)popup_list,i,&popup_object);
        if (SUCCEEDED(hr) && !popup_object) hr=E_FAIL;
        if (SUCCEEDED(hr)) { popup=(test_popup_child *)popup_object; hr=popup->vtable->get_child(popup,&child); }
        if (SUCCEEDED(hr) && child) hr=win16_xaml_visit_descendants(child,find_menu_presenter,&presenter);
        if (child) { IInspectable_Release(child); child=NULL; }
        if (popup_object) { IInspectable_Release(popup_object); popup_object=NULL; popup=NULL; }
    }
    if (SUCCEEDED(hr) && !presenter) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_NULL_EVENT");
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    VisualElement info={0}; ParentChildRelation relation={0};
    if (SUCCEEDED(hr)) {
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.MenuFlyoutPresenter"; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,presenter,&info.Handle);
    }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    unsigned int source=0,index=0;
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,presenter,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_SESSION_APPLIED_EVENT");
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,presenter,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (presenter) IInspectable_Release(presenter);
    if (popup_list) IInspectable_Release(popup_list);
    if (popup_tree) popup_tree->vtable->release(popup_tree);
    if (xaml_root) IInspectable_Release(xaml_root);
    if (SUCCEEDED(hr)) hr=flyout->vtable->hide(flyout);
    if (SUCCEEDED(hr)) hr=flyout->vtable->get_open(flyout,&open);
    if (SUCCEEDED(hr) && open) hr=E_FAIL;
    if (flyout) flyout->vtable->release(flyout);
    if (flyout_object) IInspectable_Release(flyout_object);
    if (button) button->vtable->release(button);
    if (target) target->vtable->release(target);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

static reader *menu_popup_factory;
static void CALLBACK test_menu_popup_timer(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)window; (void)message; (void)time;
    KillTimer(NULL,timer);
    HRESULT hr=test_menu_popup(menu_popup_factory);
    menu_popup_factory->vtable->release(menu_popup_factory); menu_popup_factory=NULL;
    signal_named(SUCCEEDED(hr) ? L"WIN16_XAML_PARSE_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
}

static HRESULT test_menu_presenter_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<MenuFlyoutPresenter xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation'><MenuFlyoutItem Text='First'/><MenuFlyoutItem Text='Second'/></MenuFlyoutPresenter>";
    static const wchar_t dark_markup[]=L"<MenuFlyoutPresenter xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' RequestedTheme='Dark'><MenuFlyoutItem Text='First'/><MenuFlyoutItem Text='Second'/></MenuFlyoutPresenter>";
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*scroll=NULL;
    winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL; test_window *window=NULL; test_control *control=NULL; test_item_mapping *mapping=NULL;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    const wchar_t *chosen=GetEnvironmentVariableW(L"WIN16_XAML_MENU_DARK",NULL,0) ? dark_markup : markup;
    HRESULT hr=WindowsCreateString(chosen,(UINT32)lstrlenW(chosen),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.MenuFlyoutPresenter"; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&item_mapping_iid,(void **)&mapping);
    for (INT32 i=0;SUCCEEDED(hr) && i<2;i++) {
        IInspectable *item=NULL; test_menu_item *menu_item=NULL; HSTRING text=NULL;
        hr=mapping->vtable->container_from_index(mapping,i,&item);
        if (SUCCEEDED(hr) && !item) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(item,&test_menu_item_iid,(void **)&menu_item);
        if (SUCCEEDED(hr)) hr=menu_item->vtable->get_text(menu_item,&text);
        if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(text,NULL),i ? L"Second" : L"First")) hr=E_FAIL;
        if (text) WindowsDeleteString(text);
        if (menu_item) menu_item->vtable->release(menu_item);
        if (item) IInspectable_Release(item);
    }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"MenuFlyoutPresenterScrollViewer",31,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&scroll);
    if (SUCCEEDED(hr) && !scroll) hr=E_FAIL;
    unsigned int source=0,index=0;
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (SUCCEEDED(hr)) hr=element->vtable->put_style(element,style);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IVisualTreeService_ClearProperty(service,info.Handle,index);
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (mapping) mapping->vtable->release(mapping);
    if (scroll) IInspectable_Release(scroll);
    if (root_element) root_element->vtable->release(root_element);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (control) control->vtable->release(control);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) element->vtable->release(element);
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_text_block test_text_block;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_text_block *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_text_block *);
    ULONG (STDMETHODCALLTYPE *release)(test_text_block *);
    void (STDMETHODCALLTYPE *unused_to_foreground[15])(void);
    HRESULT (STDMETHODCALLTYPE *get_foreground)(test_text_block *,IInspectable **);
    void (STDMETHODCALLTYPE *unused_to_text[7])(void);
    HRESULT (STDMETHODCALLTYPE *get_text)(test_text_block *,HSTRING *);
} test_text_block_vtable;
struct test_text_block { const test_text_block_vtable *vtable; };
static const IID test_text_block_iid={0x1ac8d84f,0x392c,0x5c7e,{0x83,0xf5,0xa5,0x3e,0x3b,0xf0,0xab,0xb0}};

typedef struct test_toggle_menu test_toggle_menu;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_toggle_menu *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_toggle_menu *);
    ULONG (STDMETHODCALLTYPE *release)(test_toggle_menu *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_checked)(test_toggle_menu *,boolean *);
    HRESULT (STDMETHODCALLTYPE *put_checked)(test_toggle_menu *,boolean);
} test_toggle_menu_vtable;
struct test_toggle_menu { const test_toggle_menu_vtable *vtable; };
static const IID test_toggle_menu_iid={0x1803f260,0x67e4,0x5bc1,{0xa6,0x3a,0x12,0x35,0x10,0x16,0x7b,0xb8}};

static HRESULT test_menu_item_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<MenuFlyoutItem xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' Text='First' KeyboardAcceleratorTextOverride='Ctrl+S'><MenuFlyoutItem.Icon><SymbolIcon Symbol='Save'/></MenuFlyoutItem.Icon></MenuFlyoutItem>";
    static const wchar_t toggle_markup[]=L"<ToggleMenuFlyoutItem xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' Text='First' KeyboardAcceleratorTextOverride='Ctrl+S'><ToggleMenuFlyoutItem.Icon><SymbolIcon Symbol='Save'/></ToggleMenuFlyoutItem.Icon></ToggleMenuFlyoutItem>";
    static const wchar_t dark_markup[]=L"<MenuFlyoutItem xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' RequestedTheme='Dark' Text='First' KeyboardAcceleratorTextOverride='Ctrl+S'><MenuFlyoutItem.Icon><SymbolIcon Symbol='Save'/></MenuFlyoutItem.Icon></MenuFlyoutItem>";
    static const wchar_t dark_toggle_markup[]=L"<ToggleMenuFlyoutItem xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' RequestedTheme='Dark' Text='First' KeyboardAcceleratorTextOverride='Ctrl+S'><ToggleMenuFlyoutItem.Icon><SymbolIcon Symbol='Save'/></ToggleMenuFlyoutItem.Icon></ToggleMenuFlyoutItem>";
    BOOL toggle_menu=GetEnvironmentVariableW(L"WIN16_XAML_TOGGLEMENU_PARSE",NULL,0)!=0;
    BOOL dark=GetEnvironmentVariableW(L"WIN16_XAML_MENU_DARK",NULL,0)!=0;
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*label=NULL,*accelerator=NULL,*icon=NULL,*icon_value=NULL;
    winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL; winui_vsm *manager=NULL;
    test_window *window=NULL; test_control *control=NULL; test_text_block *text=NULL,*shortcut=NULL; content_interface *icon_presenter=NULL; test_ui *shortcut_ui=NULL;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    const wchar_t *chosen=dark ? (toggle_menu ? dark_toggle_markup : dark_markup) : (toggle_menu ? toggle_markup : markup);
    HRESULT hr=WindowsCreateString(chosen,(UINT32)lstrlenW(chosen),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_STATES_EVENT");
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_BINDING_EVENT");
    if (SUCCEEDED(hr)) {
        info.Type=(BSTR)(toggle_menu ? L"Microsoft.UI.Xaml.Controls.ToggleMenuFlyoutItem" : L"Microsoft.UI.Xaml.Controls.MenuFlyoutItem"); relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_HANDLE_EVENT");
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_EVALUATED_EVENT");
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) signal_named(L"WIN16_XAML_LOCAL_NULL_EVENT");
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    if (SUCCEEDED(hr) && toggle_menu) {
        IInspectable *mark=NULL; test_ui *mark_ui=NULL; test_toggle_menu *toggle=NULL; HSTRING part=NULL;
        hr=WindowsCreateString(L"CheckGlyph",10,&part);
        if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,part,&mark);
        if (SUCCEEDED(hr) && !mark) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(mark,&test_ui_iid,(void **)&mark_ui);
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_toggle_menu_iid,(void **)&toggle);
        for (unsigned int i=0;SUCCEEDED(hr) && i<3;i++) {
            boolean checked=(boolean)(i==1); double opacity=-1;
            hr=toggle->vtable->put_checked(toggle,checked);
            if (SUCCEEDED(hr)) hr=mark_ui->vtable->get_opacity(mark_ui,&opacity);
            if (SUCCEEDED(hr) && opacity!=(double)checked) hr=E_FAIL;
        }
        if (toggle) toggle->vtable->release(toggle);
        if (mark_ui) mark_ui->vtable->release(mark_ui);
        if (mark) IInspectable_Release(mark);
        if (part) WindowsDeleteString(part);
    }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"TextBlock",9,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&label);
    if (SUCCEEDED(hr) && !label) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(label,&test_text_block_iid,(void **)&text);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"KeyboardAcceleratorTextBlock",28,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&accelerator);
    if (SUCCEEDED(hr) && !accelerator) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(accelerator,&test_text_block_iid,(void **)&shortcut);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(accelerator,&test_ui_iid,(void **)&shortcut_ui);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"IconContent",11,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&icon);
    if (SUCCEEDED(hr) && !icon) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(icon,&winui_presenter_iid,(void **)&icon_presenter);
    if (SUCCEEDED(hr)) hr=icon_presenter->vtable->get_content(icon_presenter,&icon_value);
    if (SUCCEEDED(hr) && !icon_value) hr=E_FAIL;
    if (SUCCEEDED(hr) && toggle_menu) {
        IInspectable *mark=NULL; test_offset *mark_offset=NULL,*icon_offset=NULL; HSTRING part=NULL;
        test_vector3 mark_position={0},icon_position={0};
        hr=((test_layout *)ui)->vtable->update_layout((test_layout *)ui);
        if (SUCCEEDED(hr)) hr=WindowsCreateString(L"CheckGlyph",10,&part);
        if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,part,&mark);
        if (SUCCEEDED(hr) && !mark) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(mark,&test_ui_iid,(void **)&mark_offset);
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(icon,&test_ui_iid,(void **)&icon_offset);
        if (SUCCEEDED(hr)) hr=mark_offset->vtable->get_offset(mark_offset,&mark_position);
        if (SUCCEEDED(hr)) hr=icon_offset->vtable->get_offset(icon_offset,&icon_position);
        if (SUCCEEDED(hr) && !(icon_position.x>mark_position.x)) hr=E_FAIL;
        if (icon_offset) icon_offset->vtable->release(icon_offset);
        if (mark_offset) mark_offset->vtable->release(mark_offset);
        if (mark) IInspectable_Release(mark);
        if (part) WindowsDeleteString(part);
    }
    HSTRING label_text=NULL,shortcut_text=NULL;
    if (SUCCEEDED(hr)) hr=text->vtable->get_text(text,&label_text);
    if (SUCCEEDED(hr)) hr=shortcut->vtable->get_text(shortcut,&shortcut_text);
    if (SUCCEEDED(hr) && (wcscmp(WindowsGetStringRawBuffer(label_text,NULL),L"First") || wcscmp(WindowsGetStringRawBuffer(shortcut_text,NULL),L"Ctrl+S"))) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.VisualStateManager",36,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    static const wchar_t *shortcut_states[]={L"KeyboardAcceleratorTextVisible",L"KeyboardAcceleratorTextCollapsed"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<2;i++) {
        HSTRING state=NULL; boolean changed=0; INT32 visibility=-1;
        hr=WindowsCreateString(shortcut_states[i],(UINT32)lstrlenW(shortcut_states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=shortcut_ui->vtable->get_visibility(shortcut_ui,&visibility);
        if (SUCCEEDED(hr) && visibility!=(INT32)i) hr=E_FAIL;
        if (state) WindowsDeleteString(state);
    }
    static const wchar_t *states[]={L"Normal",L"PointerOver",L"Disabled"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        HSTRING state=NULL; boolean changed=0; IInspectable *foreground=NULL; test_brush *brush=NULL; test_color color={0};
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=text->vtable->get_foreground(text,&foreground);
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(foreground,&brush_iid,(void **)&brush);
        if (SUCCEEDED(hr)) hr=brush->vtable->get_color(brush,&color);
        BYTE expected=i==0 ? 0 : i==1 ? 255 : 128;
        if (SUCCEEDED(hr) && (color.r!=expected || color.g!=expected || color.b!=expected)) hr=E_FAIL;
        if (SUCCEEDED(hr) && toggle_menu) {
            IInspectable *mark=NULL; HSTRING part=NULL;
            hr=WindowsCreateString(L"CheckGlyph",10,&part);
            if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,part,&mark);
            if (SUCCEEDED(hr) && !mark) hr=E_FAIL;
            if (SUCCEEDED(hr)) hr=test_shape_stroke(mark,expected);
            if (mark) IInspectable_Release(mark);
            if (part) WindowsDeleteString(part);
        }
        if (brush) brush->vtable->release(brush);
        if (foreground) IInspectable_Release(foreground);
        if (state) WindowsDeleteString(state);
    }
    unsigned int source=0,index=0;
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (SUCCEEDED(hr)) hr=element->vtable->put_style(element,style);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IVisualTreeService_ClearProperty(service,info.Handle,index);
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (icon_value) IInspectable_Release(icon_value);
    if (icon_presenter) icon_presenter->vtable->release(icon_presenter);
    if (icon) IInspectable_Release(icon);
    if (shortcut_ui) shortcut_ui->vtable->release(shortcut_ui);
    if (shortcut_text) WindowsDeleteString(shortcut_text);
    if (label_text) WindowsDeleteString(label_text);
    if (shortcut) shortcut->vtable->release(shortcut);
    if (text) text->vtable->release(text);
    if (manager) manager->vtable->release(manager);
    if (accelerator) IInspectable_Release(accelerator);
    if (label) IInspectable_Release(label);
    if (root_element) root_element->vtable->release(root_element);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (control) control->vtable->release(control);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) element->vtable->release(element);
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

typedef struct test_submenu test_submenu;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_submenu *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_submenu *);
    ULONG (STDMETHODCALLTYPE *release)(test_submenu *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    HRESULT (STDMETHODCALLTYPE *get_items)(test_submenu *,IInspectable **);
} test_submenu_vtable;
struct test_submenu { const test_submenu_vtable *vtable; };
static const IID test_submenu_iid={0x6b0688c1,0x47b0,0x53b5,{0xb6,0xf9,0x5e,0xc5,0xd6,0x62,0x3b,0x84}};
typedef struct test_vector test_vector;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(test_vector *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(test_vector *);
    ULONG (STDMETHODCALLTYPE *release)(test_vector *);
    void (STDMETHODCALLTYPE *inspectable[3])(void);
    void (STDMETHODCALLTYPE *get_at)(void);
    HRESULT (STDMETHODCALLTYPE *get_size)(test_vector *,UINT32 *);
} test_vector_vtable;
struct test_vector { const test_vector_vtable *vtable; };

static HRESULT test_submenu_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<MenuFlyoutSubItem xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' Text='More'><MenuFlyoutSubItem.Icon><SymbolIcon Symbol='Save'/></MenuFlyoutSubItem.Icon><MenuFlyoutItem Text='Child'/></MenuFlyoutSubItem>";
    static const wchar_t dark_markup[]=L"<MenuFlyoutSubItem xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' RequestedTheme='Dark' Text='More'><MenuFlyoutSubItem.Icon><SymbolIcon Symbol='Save'/></MenuFlyoutSubItem.Icon><MenuFlyoutItem Text='Child'/></MenuFlyoutSubItem>";
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*items=NULL,*dep=NULL,*root=NULL,*label=NULL,*arrow=NULL,*icon=NULL,*icon_value=NULL;
    winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL; winui_vsm *manager=NULL; test_window *window=NULL; test_control *control=NULL; test_submenu *submenu=NULL; test_text_block *text=NULL; content_interface *icon_presenter=NULL;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    const wchar_t *chosen=GetEnvironmentVariableW(L"WIN16_XAML_MENU_DARK",NULL,0) ? dark_markup : markup;
    HRESULT hr=WindowsCreateString(chosen,(UINT32)lstrlenW(chosen),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.MenuFlyoutSubItem"; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_submenu_iid,(void **)&submenu);
    if (SUCCEEDED(hr)) hr=submenu->vtable->get_items(submenu,&items);
    UINT32 count=0;
    if (SUCCEEDED(hr)) hr=((test_vector *)items)->vtable->get_size((test_vector *)items,&count);
    if (SUCCEEDED(hr) && count!=1) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"TextBlock",9,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&label);
    if (SUCCEEDED(hr) && !label) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(label,&test_text_block_iid,(void **)&text);
    HSTRING value=NULL;
    if (SUCCEEDED(hr)) hr=text->vtable->get_text(text,&value);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(value,NULL),L"More")) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"SubItemChevron",14,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&arrow);
    if (SUCCEEDED(hr) && !arrow) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"IconContent",11,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&icon);
    if (SUCCEEDED(hr) && !icon) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(icon,&winui_presenter_iid,(void **)&icon_presenter);
    if (SUCCEEDED(hr)) hr=icon_presenter->vtable->get_content(icon_presenter,&icon_value);
    if (SUCCEEDED(hr) && !icon_value) hr=E_FAIL;
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.VisualStateManager",36,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_vsm_iid,(void **)&manager);
    static const wchar_t *states[]={L"Normal",L"PointerOver",L"SubMenuOpened",L"Disabled"};
    for (unsigned int i=0;SUCCEEDED(hr) && i<ARRAYSIZE(states);i++) {
        HSTRING state=NULL; boolean changed=0; IInspectable *foreground=NULL; test_brush *brush=NULL; test_color color={0};
        hr=WindowsCreateString(states[i],(UINT32)lstrlenW(states[i]),&state);
        if (SUCCEEDED(hr)) hr=manager->vtable->go_to_state(manager,(IInspectable *)control,state,0,&changed);
        if (SUCCEEDED(hr) && !changed) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=text->vtable->get_foreground(text,&foreground);
        if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(foreground,&brush_iid,(void **)&brush);
        if (SUCCEEDED(hr)) hr=brush->vtable->get_color(brush,&color);
        BYTE expected=i==0 ? 0 : i==3 ? 128 : 255;
        if (SUCCEEDED(hr) && (color.r!=expected || color.g!=expected || color.b!=expected)) hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=test_shape_stroke(arrow,expected);
        if (brush) brush->vtable->release(brush);
        if (foreground) IInspectable_Release(foreground);
        if (state) WindowsDeleteString(state);
    }
    unsigned int source=0,index=0;
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (SUCCEEDED(hr)) hr=element->vtable->put_style(element,style);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IVisualTreeService_ClearProperty(service,info.Handle,index);
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (manager) manager->vtable->release(manager);
    if (icon_value) IInspectable_Release(icon_value);
    if (icon_presenter) icon_presenter->vtable->release(icon_presenter);
    if (icon) IInspectable_Release(icon);
    if (value) WindowsDeleteString(value);
    if (arrow) IInspectable_Release(arrow);
    if (text) text->vtable->release(text);
    if (label) IInspectable_Release(label);
    if (root_element) root_element->vtable->release(root_element);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (items) IInspectable_Release(items);
    if (submenu) submenu->vtable->release(submenu);
    if (control) control->vtable->release(control);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) element->vtable->release(element);
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

static HRESULT test_menu_separator_candidate(reader *factory,IInspectable *style)
{
    static const wchar_t markup[]=L"<MenuFlyoutSeparator xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation'/>";
    static const wchar_t dark_markup[]=L"<MenuFlyoutSeparator xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' RequestedTheme='Dark'/>";
    HSTRING name=NULL; IInspectable *object=NULL,*window_object=NULL,*ui=NULL,*dep=NULL,*root=NULL,*line=NULL;
    winui_element *element=NULL,*root_element=NULL; winui_tree *tree=NULL; test_window *window=NULL; test_control *control=NULL;
    win16_xaml_adapter adapter={0}; VisualElement info={0}; ParentChildRelation relation={0};
    const wchar_t *chosen=GetEnvironmentVariableW(L"WIN16_XAML_MENU_DARK",NULL,0) ? dark_markup : markup;
    HRESULT hr=WindowsCreateString(chosen,(UINT32)lstrlenW(chosen),&name);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,name,&object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Window",24,&name);
    if (SUCCEEDED(hr)) hr=RoActivateInstance(name,&window_object);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(window_object,&test_window_iid,(void **)&window);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&test_ui_iid,(void **)&ui);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr=window->vtable->put_content(window,ui);
    if (SUCCEEDED(hr)) hr=window->vtable->activate(window);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        info.Type=(BSTR)L"Microsoft.UI.Xaml.Controls.MenuFlyoutSeparator"; relation.Parent=1;
        hr=IXamlDiagnostics_GetHandleFromIInspectable(diagnostics,object,&info.Handle);
    }
    if (SUCCEEDED(hr)) { hr=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL); if(hr==S_FALSE)hr=E_FAIL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_control_iid,(void **)&control);
    boolean applied=0;
    if (SUCCEEDED(hr)) hr=control->vtable->apply_template(control,&applied);
    if (SUCCEEDED(hr) && !applied) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"Microsoft.UI.Xaml.Media.VisualTreeHelper",40,&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_tree_iid,(void **)&tree);
    if (name) { WindowsDeleteString(name); name=NULL; }
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(object,&winui_dependency_iid,(void **)&dep);
    if (SUCCEEDED(hr)) hr=tree->vtable->get_child(tree,dep,0,&root);
    if (SUCCEEDED(hr)) hr=IInspectable_QueryInterface(root,&winui_element_iid,(void **)&root_element);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(L"SeparatorLine",13,&name);
    if (SUCCEEDED(hr)) hr=root_element->vtable->find_name(root_element,name,&line);
    if (SUCCEEDED(hr) && !line) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=test_shape_color(line,0,0,0);
    unsigned int source=0,index=0;
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_stop(&adapter);
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceDefault) hr=E_FAIL;
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (SUCCEEDED(hr)) hr=element->vtable->put_style(element,style);
    if (SUCCEEDED(hr)) hr=win16_xaml_adapter_connect(&adapter,service,diagnostics);
    if (SUCCEEDED(hr)) {
        HRESULT observed=win16_xaml_adapter_change(&adapter,relation,info,Add,NULL);
        if (observed!=S_FALSE) hr=E_FAIL;
    }
    if (SUCCEEDED(hr)) hr=win16_xaml_button_source(service,diagnostics,object,&source,&index);
    if (SUCCEEDED(hr) && source!=BaseValueSourceLocal) hr=E_FAIL;
    if (SUCCEEDED(hr)) hr=IVisualTreeService_ClearProperty(service,info.Handle,index);
    if (adapter.session.service) (void)win16_xaml_button_set_discard(&adapter.session.buttons);
    if (adapter.service) win16_xaml_adapter_disconnect(&adapter);
    if (line) IInspectable_Release(line);
    if (root_element) root_element->vtable->release(root_element);
    if (root) IInspectable_Release(root);
    if (dep) IInspectable_Release(dep);
    if (tree) tree->vtable->release(tree);
    if (control) control->vtable->release(control);
    if (window) { (void)window->vtable->close(window); window->vtable->release(window); }
    if (element) element->vtable->release(element);
    if (ui) IInspectable_Release(ui);
    if (window_object) IInspectable_Release(window_object);
    if (object) IInspectable_Release(object);
    if (name) WindowsDeleteString(name);
    return hr;
}

static void test_template_parse(void)
{
    static const wchar_t reader_name[]=L"Microsoft.UI.Xaml.Markup.XamlReader";
    static const wchar_t password_markup[]=
        L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' "
        L"xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='PasswordBox'>"
        L"<Setter Property='Foreground' Value='Black'/><Setter Property='Background' Value='White'/>"
        L"<Setter Property='Template'><Setter.Value><ControlTemplate TargetType='PasswordBox'><Grid>"
        L"<VisualStateManager.VisualStateGroups><VisualStateGroup x:Name='CommonStates'>"
        L"<VisualState x:Name='Normal'/><VisualState x:Name='PointerOver'/><VisualState x:Name='Focused'/>"
        L"<VisualState x:Name='Disabled'><VisualState.Setters><Setter Target='ContentElement.Foreground' Value='#808080'/>"
        L"<Setter Target='PlaceholderTextContentPresenter.Foreground' Value='#808080'/></VisualState.Setters></VisualState>"
        L"</VisualStateGroup><VisualStateGroup x:Name='ButtonStates'><VisualState x:Name='ButtonCollapsed'/>"
        L"<VisualState x:Name='ButtonVisible'><VisualState.Setters><Setter Target='RevealButton.Visibility' Value='Visible'/>"
        L"</VisualState.Setters></VisualState></VisualStateGroup></VisualStateManager.VisualStateGroups>"
        L"<Grid.RowDefinitions><RowDefinition Height='Auto'/><RowDefinition Height='*'/><RowDefinition Height='Auto'/></Grid.RowDefinitions>"
        L"<Grid.ColumnDefinitions><ColumnDefinition Width='*'/><ColumnDefinition Width='Auto'/></Grid.ColumnDefinitions>"
        L"<ContentPresenter x:Name='HeaderContentPresenter' Grid.ColumnSpan='2' Content='{TemplateBinding Header}' ContentTemplate='{TemplateBinding HeaderTemplate}'/>"
        L"<Border x:Name='BorderElement' Grid.Row='1' Grid.ColumnSpan='2' Background='{TemplateBinding Background}' BorderBrush='Black' BorderThickness='1'/>"
        L"<ScrollViewer x:Name='ContentElement' Grid.Row='1' Foreground='{TemplateBinding Foreground}' Padding='{TemplateBinding Padding}' IsTabStop='False' ZoomMode='Disabled'/>"
        L"<TextBlock x:Name='PlaceholderTextContentPresenter' Grid.Row='1' Grid.ColumnSpan='2' Text='{TemplateBinding PlaceholderText}' Padding='{TemplateBinding Padding}' IsHitTestVisible='False'/>"
        L"<ToggleButton x:Name='RevealButton' Grid.Row='1' Grid.Column='1' Width='24' IsTabStop='False' Visibility='Collapsed' Content='...'>"
        L"<ToggleButton.Template><ControlTemplate TargetType='ToggleButton'><Border Background='#C0C0C0' BorderBrush='Black' BorderThickness='1'>"
        L"<VisualStateManager.VisualStateGroups><VisualStateGroup x:Name='CommonStates'><VisualState x:Name='Normal'/>"
        L"<VisualState x:Name='PointerOver'/><VisualState x:Name='Pressed'><VisualState.Setters>"
        L"<Setter Target='TopEdge.Background' Value='#808080'/><Setter Target='LeftEdge.Background' Value='#808080'/>"
        L"<Setter Target='BottomEdge.Background' Value='White'/><Setter Target='RightEdge.Background' Value='White'/>"
        L"</VisualState.Setters></VisualState><VisualState x:Name='Disabled'/></VisualStateGroup>"
        L"</VisualStateManager.VisualStateGroups><Grid><Border x:Name='TopEdge' Background='White' Height='1' VerticalAlignment='Top'/>"
        L"<Border x:Name='LeftEdge' Background='White' Width='1' HorizontalAlignment='Left'/>"
        L"<Border x:Name='BottomEdge' Background='#808080' Height='1' VerticalAlignment='Bottom'/>"
        L"<Border x:Name='RightEdge' Background='#808080' Width='1' HorizontalAlignment='Right'/>"
        L"<ContentPresenter Content='{TemplateBinding Content}' HorizontalAlignment='Center' VerticalAlignment='Center'/>"
        L"</Grid></Border></ControlTemplate></ToggleButton.Template></ToggleButton>"
        L"<ContentPresenter Grid.Row='2' Grid.ColumnSpan='2' Content='{TemplateBinding Description}'/>"
        L"</Grid></ControlTemplate></Setter.Value></Setter></Style>";

    if (!GetEnvironmentVariableW(L"WIN16_XAML_PARSE_TEST",NULL,0) ||
        InterlockedCompareExchange(&template_parse_attempted,1,0)!=0) return;
    HSTRING name=NULL,text=NULL;
    reader *factory=NULL;
    IInspectable *style=NULL;
    const wchar_t *markup=GetEnvironmentVariableW(L"WIN16_XAML_SEPARATOR_PARSE",NULL,0)
        ? win16_xaml_menu_separator_template() : GetEnvironmentVariableW(L"WIN16_XAML_SUBMENU_PARSE",NULL,0)
        ? win16_xaml_submenu_template() : GetEnvironmentVariableW(L"WIN16_XAML_TOGGLEMENU_PARSE",NULL,0)
        ? win16_xaml_toggle_menu_template() : GetEnvironmentVariableW(L"WIN16_XAML_MENUITEM_PARSE",NULL,0)
        ? win16_xaml_menu_item_template() : GetEnvironmentVariableW(L"WIN16_XAML_MENU_PARSE",NULL,0)
        ? win16_xaml_menu_presenter_template() : GetEnvironmentVariableW(L"WIN16_XAML_TOGGLEBUTTON_PARSE",NULL,0)
        ? win16_xaml_toggle_button_template() : GetEnvironmentVariableW(L"WIN16_XAML_PASSWORD_PARSE",NULL,0)
        ? password_markup : GetEnvironmentVariableW(L"WIN16_XAML_SLIDER_PARSE",NULL,0)
        ? win16_xaml_slider_template() : GetEnvironmentVariableW(L"WIN16_XAML_TOGGLE_PARSE",NULL,0)
        ? win16_xaml_toggle_template() : GetEnvironmentVariableW(L"WIN16_XAML_PROGRESS_TEST",NULL,0)
        ? win16_xaml_progress_template() : GetEnvironmentVariableW(L"WIN16_XAML_LIST_TEST",NULL,0)
        ? win16_xaml_list_template() : GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_TEST",NULL,0)
        ? win16_xaml_scroll_template() : GetEnvironmentVariableW(L"WIN16_XAML_COMBO_TEST",NULL,0)
        ? win16_xaml_combo_template() : GetEnvironmentVariableW(L"WIN16_XAML_TEXTBOX_TEST",NULL,0)
        ? win16_xaml_textbox_template() : GetEnvironmentVariableW(L"WIN16_XAML_RADIO_TEST",NULL,0)
        ? win16_xaml_radio_template() : GetEnvironmentVariableW(L"WIN16_XAML_CHECKBOX_TEST",NULL,0)
        ? win16_xaml_checkbox_template() : win16_xaml_button_template();
    HRESULT hr=WindowsCreateString(reader_name,(UINT32)(ARRAYSIZE(reader_name)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&winui_reader_iid,(void **)&factory);
    if (SUCCEEDED(hr)) hr=WindowsCreateString(markup,(UINT32)lstrlenW(markup),&text);
    if (SUCCEEDED(hr)) hr=factory->vtable->load(factory,text,&style);
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_SEPARATOR_PARSE",NULL,0))
        hr=test_menu_separator_candidate(factory,style);
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_SUBMENU_PARSE",NULL,0))
        hr=test_submenu_candidate(factory,style);
    if (SUCCEEDED(hr) && style && (GetEnvironmentVariableW(L"WIN16_XAML_MENUITEM_PARSE",NULL,0) || GetEnvironmentVariableW(L"WIN16_XAML_TOGGLEMENU_PARSE",NULL,0)))
        hr=test_menu_item_candidate(factory,style);
    BOOL popup=GetEnvironmentVariableW(L"WIN16_XAML_MENU_POPUP",NULL,0)!=0;
    if (SUCCEEDED(hr) && style && popup) {
        menu_popup_factory=factory; factory->vtable->add_ref(factory);
        if (!SetTimer(NULL,0,100,test_menu_popup_timer)) {
            menu_popup_factory->vtable->release(menu_popup_factory); menu_popup_factory=NULL;
            hr=HRESULT_FROM_WIN32(GetLastError());
        }
    }
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_MENU_PARSE",NULL,0))
        hr=test_menu_presenter_candidate(factory,style);
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_TOGGLEBUTTON_PARSE",NULL,0))
        hr=test_toggle_button_candidate(factory,style);
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_PASSWORD_PARSE",NULL,0))
        hr=test_password_candidate(factory,style);
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_SLIDER_PARSE",NULL,0))
        hr=test_slider_candidate(factory,style);
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_TOGGLE_PARSE",NULL,0))
        hr=test_switch_candidate(factory,style);
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_PROGRESS_TEST",NULL,0)) {
        progress_factory=factory; factory->vtable->add_ref(factory);
        progress_style=style; IInspectable_AddRef(style);
        if (!SetTimer(NULL,0,100,test_progress_start_timer)) {
            IInspectable_Release(progress_style); progress_style=NULL;
            progress_factory->vtable->release(progress_factory); progress_factory=NULL;
            signal_named(L"WIN16_XAML_FAILED_EVENT");
        }
    }
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_LIST_TEST",NULL,0)) {
        HRESULT list=test_list_candidate(factory,style);
        signal_named(SUCCEEDED(list) ? L"WIN16_XAML_LIST_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    }
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_TEST",NULL,0)) {
        HRESULT scroll=test_scroll_candidate(style,0);
        if (!GetEnvironmentVariableW(L"WIN16_XAML_SCROLL_ASYNC",NULL,0)) {
            if (SUCCEEDED(scroll)) scroll=test_scroll_candidate(style,1);
            signal_named(SUCCEEDED(scroll) ? L"WIN16_XAML_SCROLL_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
        } else if (FAILED(scroll)) signal_named(L"WIN16_XAML_FAILED_EVENT");
    }
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_COMBO_TEST",NULL,0)) {
        combo_factory=factory; factory->vtable->add_ref(factory);
        combo_style=style; IInspectable_AddRef(style);
        if (!SetTimer(NULL,0,100,test_combo_timer)) {
            IInspectable_Release(combo_style); combo_style=NULL;
            combo_factory->vtable->release(combo_factory); combo_factory=NULL;
            signal_named(L"WIN16_XAML_FAILED_EVENT");
        }
    }
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_TEXTBOX_TEST",NULL,0)) {
        HRESULT candidate=test_textbox_candidate();
        signal_named(SUCCEEDED(candidate) ? L"WIN16_XAML_TEXTBOX_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    }
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_CHECKBOX_TEST",NULL,0)) {
        HRESULT checkbox=test_checkbox_apply();
        signal_named(SUCCEEDED(checkbox) ? L"WIN16_XAML_CHECKBOX_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    }
    if (SUCCEEDED(hr) && style && GetEnvironmentVariableW(L"WIN16_XAML_RADIO_TEST",NULL,0)) {
        HRESULT radio=test_radio_apply();
        if (SUCCEEDED(radio)) radio=test_radio_group();
        signal_named(SUCCEEDED(radio) ? L"WIN16_XAML_RADIO_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
    }
    if (!popup || FAILED(hr)) signal_named(SUCCEEDED(hr) && style ? L"WIN16_XAML_PARSE_OK_EVENT" : L"WIN16_XAML_FAILED_EVENT");
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
