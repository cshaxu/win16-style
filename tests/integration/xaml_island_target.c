/* Self-owned UWP XAML island fixture. No styling or target-process hooks. */
#define COBJMACROS
#include <windows.h>
#include <roapi.h>
#include <winstring.h>
#include <inspectable.h>
#include <xamlom.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <stdio.h>
#include <wchar.h>
#include "lib-win32/win16-control-xaml/diagnostics_start.h"

typedef struct self_test { WCHAR tap[32768]; HANDLE ready,done,stop; HRESULT result; } self_test;
static DWORD WINAPI self_worker(void *parameter)
{
    self_test *test=parameter;
    static const CLSID clsid={0x7132a744,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};
    HRESULT apartment=RoInitialize(RO_INIT_MULTITHREADED);
    HMODULE module=LoadLibraryW(L"Windows.UI.Xaml.dll");
    test->result=FAILED(apartment) ? apartment : !module ? HRESULT_FROM_WIN32(GetLastError()) :
        win16_xaml_diagnostics_start(GetCurrentProcessId(),L"Windows.UI.Xaml.dll",NULL,test->tap,L"VisualDiagConnection1",&clsid);
    SetEvent(test->done);
    if (SUCCEEDED(test->result)) (void)WaitForSingleObject(test->stop,5000);
    if (module) FreeLibrary(module);
    if (SUCCEEDED(apartment)) RoUninitialize();
    return 0;
}
static HRESULT exercise_self_diagnostics(void)
{
    self_test *test=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*test));
    if (!test) return E_OUTOFMEMORY;
    DWORD n=GetEnvironmentVariableW(L"WIN16_XAML_SELF_TAP",test->tap,ARRAYSIZE(test->tap));
    if (!n || n>=ARRAYSIZE(test->tap)) { HeapFree(GetProcessHeap(),0,test); return E_INVALIDARG; }
    WCHAR event_name[100];
    swprintf(event_name,ARRAYSIZE(event_name),L"Local\\w16-self-ready-%lu",(unsigned long)GetCurrentProcessId());
    test->ready=CreateEventW(NULL,TRUE,FALSE,event_name);
    test->done=CreateEventW(NULL,TRUE,FALSE,NULL); test->stop=CreateEventW(NULL,TRUE,FALSE,NULL);
    if (!test->ready || !test->done || !test->stop) {
        if (test->ready) CloseHandle(test->ready);
        if (test->done) CloseHandle(test->done);
        if (test->stop) CloseHandle(test->stop);
        HeapFree(GetProcessHeap(),0,test); return E_OUTOFMEMORY;
    }
    SetEnvironmentVariableW(L"WIN16_XAML_GENERIC_SITE_TEST",L"1");
    SetEnvironmentVariableW(L"WIN16_XAML_ASYNC_SITE_TEST",L"1");
    SetEnvironmentVariableW(L"WIN16_XAML_SITE_READY_EVENT",event_name);
    HANDLE worker=CreateThread(NULL,0,self_worker,test,0,NULL);
    if (!worker) {
        HRESULT error=HRESULT_FROM_WIN32(GetLastError());
        CloseHandle(test->ready); CloseHandle(test->done); CloseHandle(test->stop);
        HeapFree(GetProcessHeap(),0,test); return error;
    }
    DWORD deadline=GetTickCount()+5000;
    while (GetTickCount()<deadline && WaitForSingleObject(test->ready,0)!=WAIT_OBJECT_0 &&
           WaitForSingleObject(worker,0)!=WAIT_OBJECT_0) {
        MSG message;
        while (PeekMessageW(&message,NULL,0,0,PM_REMOVE)) { TranslateMessage(&message); DispatchMessageW(&message); }
        (void)MsgWaitForMultipleObjects(1,&test->ready,FALSE,20,QS_ALLINPUT);
    }
    BOOL ready=WaitForSingleObject(test->ready,0)==WAIT_OBJECT_0;
    BOOL done=WaitForSingleObject(test->done,0)==WAIT_OBJECT_0;
    HRESULT result=done ? test->result : HRESULT_FROM_WIN32(ERROR_TIMEOUT);
    printf("self_initialize=%08lx site_ready=%d\n",(unsigned long)result,ready);
    SetEvent(test->stop);
    if (WaitForSingleObject(worker,2000)==WAIT_OBJECT_0) {
        CloseHandle(worker); CloseHandle(test->ready); CloseHandle(test->done); CloseHandle(test->stop);
        HeapFree(GetProcessHeap(),0,test);
    }
    SetEnvironmentVariableW(L"WIN16_XAML_GENERIC_SITE_TEST",NULL);
    SetEnvironmentVariableW(L"WIN16_XAML_ASYNC_SITE_TEST",NULL);
    SetEnvironmentVariableW(L"WIN16_XAML_SITE_READY_EVENT",NULL);
    return FAILED(result) ? result : ready ? S_OK : HRESULT_FROM_WIN32(ERROR_TIMEOUT);
}

typedef struct xaml_source xaml_source;
typedef struct xaml_source_vtable {
    HRESULT (STDMETHODCALLTYPE *query)(xaml_source *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(xaml_source *);
    ULONG (STDMETHODCALLTYPE *release)(xaml_source *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(xaml_source *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(xaml_source *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(xaml_source *, TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *get_content)(xaml_source *, IInspectable **);
    HRESULT (STDMETHODCALLTYPE *put_content)(xaml_source *, IInspectable *);
} xaml_source_vtable;
struct xaml_source { const xaml_source_vtable *vtable; };

/* Public WinRT ABI: IFrameworkElement methods before get_Style are not called. */
typedef struct framework_element framework_element;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(framework_element *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(framework_element *);
    ULONG (STDMETHODCALLTYPE *release)(framework_element *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(framework_element *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(framework_element *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(framework_element *, TrustLevel *);
    void (STDMETHODCALLTYPE *unused[32])(void);
    HRESULT (STDMETHODCALLTYPE *get_style)(framework_element *, IInspectable **);
    HRESULT (STDMETHODCALLTYPE *put_style)(framework_element *, IInspectable *);
} framework_vtable;
struct framework_element { const framework_vtable *vtable; };

typedef struct xaml_reader xaml_reader;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(xaml_reader *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(xaml_reader *);
    ULONG (STDMETHODCALLTYPE *release)(xaml_reader *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(xaml_reader *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(xaml_reader *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(xaml_reader *, TrustLevel *);
    HRESULT (STDMETHODCALLTYPE *load)(xaml_reader *, HSTRING, IInspectable **);
} reader_vtable;
struct xaml_reader { const reader_vtable *vtable; };

typedef struct xaml_control xaml_control;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(xaml_control *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(xaml_control *);
    ULONG (STDMETHODCALLTYPE *release)(xaml_control *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(xaml_control *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(xaml_control *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(xaml_control *, TrustLevel *);
    void (STDMETHODCALLTYPE *unused[39])(void);
    HRESULT (STDMETHODCALLTYPE *apply_template)(xaml_control *, boolean *);
} control_vtable;
struct xaml_control { const control_vtable *vtable; };

typedef struct visual_tree visual_tree;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(visual_tree *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(visual_tree *);
    ULONG (STDMETHODCALLTYPE *release)(visual_tree *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(visual_tree *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(visual_tree *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(visual_tree *, TrustLevel *);
    void (STDMETHODCALLTYPE *unused[4])(void);
    HRESULT (STDMETHODCALLTYPE *get_child)(visual_tree *, IInspectable *, INT32, IInspectable **);
    HRESULT (STDMETHODCALLTYPE *get_count)(visual_tree *, IInspectable *, INT32 *);
    HRESULT (STDMETHODCALLTYPE *get_parent)(visual_tree *, IInspectable *, IInspectable **);
} visual_tree_vtable;
struct visual_tree { const visual_tree_vtable *vtable; };

static const IID source_iid = {0xd585bfe1,0x00ff,0x51be,{0xba,0x1d,0xa1,0x32,0x99,0x56,0xea,0x0a}};
static const IID element_iid = {0x676d0be9,0xb65c,0x41c6,{0xba,0x40,0x58,0xcf,0x87,0xf2,0x01,0xc1}};
static const IID native_iid = {0x3cbcf1bf,0x2f76,0x4e9c,{0x96,0xab,0xe8,0x4b,0x37,0x97,0x25,0x54}};
static const IID unknown_iid = {0x00000000,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const IID framework_iid = {0xa391d09b,0x4a99,0x4b7c,{0x9d,0x8d,0x6f,0xa5,0xd0,0x1f,0x6f,0xbf}};
static const IID reader_iid = {0x9891c6bd,0x534f,0x4955,{0xb8,0x5a,0x8a,0x8d,0xc0,0xdc,0xa6,0x02}};
static const IID style_iid = {0xc4a9f225,0x9db7,0x4a7d,{0xb6,0xd1,0xf7,0x4e,0xdb,0x92,0x93,0xc2}};
static const IID control_iid = {0xa8912263,0x2951,0x4f58,{0xa9,0xc5,0x5a,0x13,0x4e,0xaa,0x7f,0x07}};
static const IID tree_iid = {0xe75758c4,0xd25d,0x4b1d,{0x97,0x1f,0x59,0x6f,0x17,0xf1,0x2b,0xaa}};
static const IID dependency_iid = {0x5c526665,0xf60e,0x4912,{0xaf,0x59,0x5f,0xe0,0x68,0x0f,0x08,0x9d}};
static const CLSID tap_clsid = {0x7132a744,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};

static void pump_messages(void)
{
    DWORD until = GetTickCount() + 250;
    MSG message;
    while ((LONG)(until - GetTickCount()) > 0) {
        while (PeekMessageW(&message,NULL,0,0,PM_REMOVE)) {
            TranslateMessage(&message); DispatchMessageW(&message);
        }
        MsgWaitForMultipleObjects(0,NULL,FALSE,25,QS_ALLINPUT);
    }
}

static HRESULT passive_wait(void)
{
    WCHAR ready_name[160], stop_name[160];
    DWORD a = GetEnvironmentVariableW(L"WIN16_XAML_READY_EVENT",ready_name,ARRAYSIZE(ready_name));
    DWORD b = GetEnvironmentVariableW(L"WIN16_XAML_STOP_EVENT",stop_name,ARRAYSIZE(stop_name));
    if (!a || a >= ARRAYSIZE(ready_name) || !b || b >= ARRAYSIZE(stop_name)) return E_INVALIDARG;
    HANDLE ready = OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);
    HANDLE stop = OpenEventW(SYNCHRONIZE,FALSE,stop_name);
    if (!ready || !stop) { if (ready) CloseHandle(ready); if (stop) CloseHandle(stop); return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE); }
    SetEvent(ready);
    DWORD until = GetTickCount() + 8000;
    MSG message;
    while ((LONG)(until - GetTickCount()) > 0) {
        while (PeekMessageW(&message,NULL,0,0,PM_REMOVE)) { TranslateMessage(&message); DispatchMessageW(&message); }
        if (MsgWaitForMultipleObjects(1,&stop,FALSE,25,QS_ALLINPUT) == WAIT_OBJECT_0) break;
    }
    CloseHandle(ready); CloseHandle(stop);
    return S_OK;
}

static HRESULT exercise_style(IInspectable *button);

static HRESULT check_diagnostics(xaml_source *source, IInspectable *element)
{
    typedef HRESULT (WINAPI *initialize_fn)(PCWSTR,DWORD,PCWSTR,PCWSTR,CLSID,PCWSTR);
    typedef LONG (WINAPI *seen_fn)(void);
    typedef void (WINAPI *stop_fn)(void);
    typedef HRESULT (WINAPI *roundtrip_fn)(IInspectable *);
    typedef HRESULT (WINAPI *style_source_fn)(IInspectable *,unsigned int *,unsigned int *,unsigned int *);
    typedef HRESULT (WINAPI *clear_style_fn)(IInspectable *,unsigned int);
    typedef HRESULT (WINAPI *get_button_fn)(IInspectable **);
    typedef HRESULT (WINAPI *hit_test_fn)(unsigned int *,unsigned int *);
    WCHAR path[32768];
    WCHAR endpoint[80] = L"VisualDiagConnection1";
    DWORD endpoint_length = GetEnvironmentVariableW(L"WIN16_XAML_TEST_ENDPOINT",endpoint,ARRAYSIZE(endpoint));
    if (endpoint_length >= ARRAYSIZE(endpoint)) return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    if (!endpoint_length) wcscpy(endpoint,L"VisualDiagConnection1");
    DWORD length = GetEnvironmentVariableW(L"WIN16_XAML_TEST_TAP",path,ARRAYSIZE(path));
    if (!length || length >= ARRAYSIZE(path)) return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    HMODULE tap = LoadLibraryW(path), xaml = GetModuleHandleW(L"Windows.UI.Xaml.dll");
    if (!tap || !xaml) return HRESULT_FROM_WIN32(GetLastError());
    initialize_fn initialize = (initialize_fn)(void *)GetProcAddress(xaml,"InitializeXamlDiagnosticsEx");
    seen_fn seen = (seen_fn)(void *)GetProcAddress(tap,"TestSawButton");
    seen_fn site_seen = (seen_fn)(void *)GetProcAddress(tap,"TestSiteSeen");
    seen_fn changes = (seen_fn)(void *)GetProcAddress(tap,"TestChanges");
    stop_fn stop = (stop_fn)(void *)GetProcAddress(tap,"TestStop");
    roundtrip_fn roundtrip = (roundtrip_fn)(void *)GetProcAddress(tap,"TestRoundtrip");
    style_source_fn style_source = (style_source_fn)(void *)GetProcAddress(tap,"TestStyleSource");
    clear_style_fn clear_style = (clear_style_fn)(void *)GetProcAddress(tap,"TestClearStyle");
    get_button_fn get_button = (get_button_fn)(void *)GetProcAddress(tap,"TestGetButton");
    hit_test_fn hit_test = (hit_test_fn)(void *)GetProcAddress(tap,"TestHitTest");
    HRESULT hr = initialize && seen && site_seen && changes && stop && roundtrip && style_source && clear_style && get_button && hit_test ? S_OK : HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
    if (SUCCEEDED(hr)) hr = initialize(endpoint,GetCurrentProcessId(),NULL,path,tap_clsid,NULL);
    printf("diagnostics_initialize=%08lx\n",(unsigned long)hr);
    if (SUCCEEDED(hr)) {
        HRESULT detach = source->vtable->put_content(source,NULL);
        HRESULT reattach = source->vtable->put_content(source,element);
        printf("diagnostics_remount=%08lx/%08lx\n",(unsigned long)detach,(unsigned long)reattach);
        if (FAILED(detach)) hr = detach;
        else if (FAILED(reattach)) hr = reattach;
        DWORD until = GetTickCount() + 3000;
        MSG message;
        while (SUCCEEDED(hr) && !seen() && (LONG)(until - GetTickCount()) > 0) {
            while (PeekMessageW(&message,NULL,0,0,PM_REMOVE)) {
                TranslateMessage(&message); DispatchMessageW(&message);
            }
            MsgWaitForMultipleObjects(0,NULL,FALSE,25,QS_ALLINPUT);
        }
        printf("diagnostics_site=%ld changes=%ld button=%ld\n",(long)site_seen(),(long)changes(),(long)seen());
        unsigned int hit_total = 0, hit_buttons = 0;
        HRESULT hit_hr = hit_test(&hit_total,&hit_buttons);
        printf("diagnostics_hit=%08lx total=%u buttons=%u\n",(unsigned long)hit_hr,hit_total,hit_buttons);
        HRESULT roundtrip_hr = roundtrip(element);
        printf("diagnostics_roundtrip=%08lx\n",(unsigned long)roundtrip_hr);
        unsigned int matches = 0, base = 0, index = 0;
        HRESULT source_hr = style_source(element,&matches,&base,&index);
        printf("diagnostics_style_source=%08lx matches=%u base=%u index=%u\n",(unsigned long)source_hr,matches,base,index);
        BOOL default_source = SUCCEEDED(source_hr) && matches == 1 && base == BaseValueSourceDefault;
        IInspectable *discovered = NULL;
        HRESULT exercise_hr = get_button(&discovered);
        if (SUCCEEDED(exercise_hr)) exercise_hr = exercise_style(discovered);
        printf("diagnostics_style_exercise=%08lx\n",(unsigned long)exercise_hr);
        printf("diagnostics_button_after_style=%ld\n",(long)seen());
        matches = 0; base = 0;
        source_hr = style_source(element,&matches,&base,&index);
        printf("diagnostics_style_after=%08lx matches=%u base=%u\n",(unsigned long)source_hr,matches,base);
        if (default_source && SUCCEEDED(exercise_hr) && SUCCEEDED(source_hr) && matches == 1 && base == BaseValueSourceLocal) {
            HRESULT clear_hr = clear_style(element,index);
            if (SUCCEEDED(clear_hr)) clear_hr = style_source(element,&matches,&base,&index);
            printf("diagnostics_style_clear=%08lx matches=%u base=%u\n",(unsigned long)clear_hr,matches,base);
        }
        if (discovered) IInspectable_Release(discovered);
        if (SUCCEEDED(hr) && FAILED(exercise_hr)) hr = exercise_hr;
        stop();
    }
    printf("diagnostics=%08lx\n",(unsigned long)hr);
    /* The diagnostic runtime may retain callbacks: keep this test DLL loaded to process exit. */
    return hr;
}

static HRESULT same_identity(IInspectable *left, IInspectable *right, BOOL *same)
{
    IUnknown *a = NULL, *b = NULL;
    *same = left == NULL && right == NULL;
    if (!left || !right) return S_OK;
    HRESULT hr = IInspectable_QueryInterface(left,&unknown_iid,(void **)&a);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(right,&unknown_iid,(void **)&b);
    if (SUCCEEDED(hr)) *same = a == b;
    if (a) IUnknown_Release(a);
    if (b) IUnknown_Release(b);
    return hr;
}

static HRESULT check_template_tree(IInspectable *button)
{
    static const wchar_t class_name[] = L"Windows.UI.Xaml.Media.VisualTreeHelper";
    HSTRING name = NULL, child_name = NULL;
    visual_tree *tree = NULL;
    IInspectable *dependency = NULL, *child = NULL;
    INT32 count = 0;
    HRESULT hr = WindowsCreateString(class_name,(UINT32)(ARRAYSIZE(class_name)-1),&name);
    if (SUCCEEDED(hr)) hr = RoGetActivationFactory(name,&tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(button,&dependency_iid,(void **)&dependency);
    if (SUCCEEDED(hr)) hr = tree->vtable->get_count(tree,dependency,&count);
    if (SUCCEEDED(hr) && count != 1) hr = E_FAIL;
    if (SUCCEEDED(hr)) hr = tree->vtable->get_child(tree,dependency,0,&child);
    if (SUCCEEDED(hr)) hr = IInspectable_GetRuntimeClassName(child,&child_name);
    if (SUCCEEDED(hr) && wcscmp(WindowsGetStringRawBuffer(child_name,NULL),L"Windows.UI.Xaml.Controls.Border")) hr = E_FAIL;
    if (child_name) WindowsDeleteString(child_name);
    if (child) IInspectable_Release(child);
    if (dependency) IInspectable_Release(dependency);
    if (tree) tree->vtable->release(tree);
    if (name) WindowsDeleteString(name);
    return hr;
}

static void report_parent(IInspectable *button)
{
    static const wchar_t class_name[] = L"Windows.UI.Xaml.Media.VisualTreeHelper";
    HSTRING name = NULL, parent_name = NULL;
    visual_tree *tree = NULL;
    IInspectable *dependency = NULL, *parent = NULL, *current = NULL;
    HRESULT hr = WindowsCreateString(class_name,(UINT32)(ARRAYSIZE(class_name)-1),&name);
    if (SUCCEEDED(hr)) hr = RoGetActivationFactory(name,&tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(button,&dependency_iid,(void **)&dependency);
    if (SUCCEEDED(hr)) { current = dependency; IInspectable_AddRef(current); }
    for (unsigned int depth=0;SUCCEEDED(hr) && current && depth<24;depth++) {
        hr = tree->vtable->get_parent(tree,current,&parent);
        if (SUCCEEDED(hr) && parent) hr = IInspectable_GetRuntimeClassName(parent,&parent_name);
        if (parent_name) { fwprintf(stderr,L"visual_ancestor_%u=%ls\n",depth,WindowsGetStringRawBuffer(parent_name,NULL)); WindowsDeleteString(parent_name); parent_name=NULL; }
        IInspectable_Release(current); current=parent; parent=NULL;
    }
    printf("visual_parent_chain_hr=%08lx\n",(unsigned long)hr);
    if (current) IInspectable_Release(current);
    if (parent) IInspectable_Release(parent);
    if (dependency) IInspectable_Release(dependency);
    if (tree) tree->vtable->release(tree);
    if (name) WindowsDeleteString(name);
}

static HRESULT exercise_style(IInspectable *button)
{
    static const wchar_t markup[] =
        L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' "
        L"xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='Button'>"
        L"<Setter Property='Template'><Setter.Value><ControlTemplate TargetType='Button'>"
        L"<Border Background='Silver' BorderBrush='Black' BorderThickness='1'>"
        L"<ContentPresenter HorizontalAlignment='Center' VerticalAlignment='Center'/>"
        L"</Border></ControlTemplate></Setter.Value></Setter></Style>";
    HSTRING name = NULL, text = NULL;
    IInspectable *object = NULL, *style = NULL, *previous = NULL, *current = NULL, *after = NULL;
    framework_element *element = NULL;
    xaml_reader *reader = NULL;
    xaml_control *control = NULL;
    boolean templated = 0;
    BOOL applied = FALSE, same = FALSE;
    const wchar_t reader_name[] = L"Windows.UI.Xaml.Markup.XamlReader";
    HRESULT hr = WindowsCreateString(reader_name,(UINT32)(ARRAYSIZE(reader_name)-1),&name);
    if (SUCCEEDED(hr)) hr = RoGetActivationFactory(name,&reader_iid,(void **)&reader);
    if (SUCCEEDED(hr)) hr = WindowsCreateString(markup,(UINT32)(ARRAYSIZE(markup)-1),&text);
    if (SUCCEEDED(hr)) hr = reader->vtable->load(reader,text,&object);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(object,&style_iid,(void **)&style);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(button,&framework_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(button,&control_iid,(void **)&control);
    if (SUCCEEDED(hr)) hr = element->vtable->get_style(element,&previous);
    if (SUCCEEDED(hr)) { hr = element->vtable->put_style(element,style); applied = SUCCEEDED(hr); }
    if (SUCCEEDED(hr)) hr = element->vtable->get_style(element,&current);
    if (SUCCEEDED(hr)) hr = same_identity(current,style,&same);
    if (SUCCEEDED(hr) && !same) hr = E_FAIL;
    if (SUCCEEDED(hr)) hr = control->vtable->apply_template(control,&templated);
    if (SUCCEEDED(hr) && !templated) hr = E_FAIL;
    if (SUCCEEDED(hr)) pump_messages();
    if (SUCCEEDED(hr)) hr = check_template_tree(button);
    if (applied) {
        HRESULT restored = element->vtable->put_style(element,previous);
        if (SUCCEEDED(restored)) restored = element->vtable->get_style(element,&after);
        if (SUCCEEDED(restored)) restored = same_identity(previous,after,&same);
        if (SUCCEEDED(restored) && !same) restored = E_FAIL;
        if (SUCCEEDED(hr)) hr = restored;
    }
    if (after) IInspectable_Release(after);
    if (current) IInspectable_Release(current);
    if (previous) IInspectable_Release(previous);
    if (element) element->vtable->release(element);
    if (control) control->vtable->release(control);
    if (style) IInspectable_Release(style);
    if (object) IInspectable_Release(object);
    if (reader) reader->vtable->release(reader);
    if (text) WindowsDeleteString(text);
    if (name) WindowsDeleteString(name);
    return hr;
}

static HRESULT activate(const wchar_t *name, IInspectable **object)
{
    HSTRING text = NULL;
    HRESULT hr = WindowsCreateString(name, (UINT32)lstrlenW(name), &text);
    if (SUCCEEDED(hr)) hr = RoActivateInstance(text, object);
    if (text) WindowsDeleteString(text);
    return hr;
}

int main(void)
{
    HDESK original = GetThreadDesktop(GetCurrentThreadId());
    WCHAR desktop_name[64];
    swprintf(desktop_name,ARRAYSIZE(desktop_name),L"Win16XamlFixture-%lu",(unsigned long)GetCurrentProcessId());
    HDESK desktop = CreateDesktopW(desktop_name,NULL,NULL,0,GENERIC_ALL,NULL);
    if (!desktop) { printf("CreateDesktop error=%lu\n",(unsigned long)GetLastError()); return 77; }
    if (!SetThreadDesktop(desktop)) {
        printf("SetThreadDesktop error=%lu\n",(unsigned long)GetLastError());
        CloseDesktop(desktop); return 77;
    }
    HRESULT hr = RoInitialize(RO_INIT_SINGLETHREADED);
    HWND window = NULL, child = NULL;
    IInspectable *instance = NULL, *button = NULL, *element = NULL, *actual = NULL;
    xaml_source *source = NULL;
    IDesktopWindowXamlSourceNative *native = NULL;
    IUnknown *expected_identity = NULL, *actual_identity = NULL;
    BOOL same = FALSE;
    if (FAILED(hr)) { SetThreadDesktop(original); CloseDesktop(desktop); return 1; }
    window = CreateWindowExW(0,L"STATIC",L"XAML test fixture",WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT,CW_USEDEFAULT,320,200,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if (!window) { hr = HRESULT_FROM_WIN32(GetLastError()); goto done; }
    hr = activate(L"Windows.UI.Xaml.Hosting.DesktopWindowXamlSource", &instance);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(instance,&native_iid,(void **)&native);
    if (SUCCEEDED(hr)) hr = IDesktopWindowXamlSourceNative_AttachToWindow(native,window);
    if (SUCCEEDED(hr)) hr = IDesktopWindowXamlSourceNative_get_WindowHandle(native,&child);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(instance,&source_iid,(void **)&source);
    if (SUCCEEDED(hr)) hr = activate(L"Windows.UI.Xaml.Controls.Button",&button);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(button,&element_iid,(void **)&element);
    if (SUCCEEDED(hr)) hr = source->vtable->put_content(source,element);
    if (SUCCEEDED(hr)) hr = source->vtable->get_content(source,&actual);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(button,&unknown_iid,(void **)&expected_identity);
    if (SUCCEEDED(hr)) hr = IInspectable_QueryInterface(actual,&unknown_iid,(void **)&actual_identity);
    if (SUCCEEDED(hr)) same = expected_identity == actual_identity;
    if (SUCCEEDED(hr) && same) {
        RECT client;
        if (!GetClientRect(window,&client) ||
            !SetWindowPos(child,NULL,0,0,client.right,client.bottom,SWP_NOZORDER | SWP_SHOWWINDOW))
            hr = HRESULT_FROM_WIN32(GetLastError());
        else {
            ShowWindow(window,SW_SHOWNOACTIVATE);
            pump_messages();
            if (GetEnvironmentVariableW(L"WIN16_XAML_SELF_TAP",NULL,0)) hr=exercise_self_diagnostics();
            else if (GetEnvironmentVariableW(L"WIN16_XAML_FIXTURE_ONLY",NULL,0)) pump_messages();
            else if (GetEnvironmentVariableW(L"WIN16_XAML_PASSIVE",NULL,0)) hr = passive_wait();
            else {
                report_parent(button);
                hr = check_diagnostics(source,element);
                if (SUCCEEDED(hr)) hr = exercise_style(button);
            }
        }
    }
    printf("hr=%08lx child=%d same=%d\n",(unsigned long)hr,child!=NULL,same);
done:
    if (actual_identity) IUnknown_Release(actual_identity);
    if (expected_identity) IUnknown_Release(expected_identity);
    if (actual) IInspectable_Release(actual);
    if (source) { source->vtable->put_content(source,NULL); source->vtable->release(source); }
    if (element) IInspectable_Release(element);
    if (button) IInspectable_Release(button);
    if (native) IDesktopWindowXamlSourceNative_Release(native);
    if (instance) IInspectable_Release(instance);
    if (window) DestroyWindow(window);
    RoUninitialize();
    SetThreadDesktop(original);
    CloseDesktop(desktop);
    return FAILED(hr) || !child || !same ? 2 : 0;
}
