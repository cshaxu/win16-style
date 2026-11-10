#define COBJMACROS
#include "adapter.h"
#include <wchar.h>
static _Thread_local BOOL scanning_root;

static void release_interfaces(win16_xaml_adapter *adapter)
{
    if (adapter->diagnostics) IXamlDiagnostics_Release(adapter->diagnostics);
    if (adapter->service) IVisualTreeService_Release(adapter->service);
    adapter->diagnostics=NULL; adapter->service=NULL;
}
HRESULT win16_xaml_adapter_connect(win16_xaml_adapter *adapter,IVisualTreeService *service,IXamlDiagnostics *diagnostics)
{
    if (!adapter || !service || !diagnostics || adapter->service || adapter->diagnostics) return E_INVALIDARG;
    IVisualTreeService_AddRef(service); IXamlDiagnostics_AddRef(diagnostics);
    adapter->service=service; adapter->diagnostics=diagnostics; adapter->connected=TRUE;
    InterlockedExchange(&adapter->stopping,0);
    return S_OK;
}
HRESULT win16_xaml_adapter_change(win16_xaml_adapter *adapter,ParentChildRelation relation,
                                  VisualElement element,VisualMutationType mutation,IInspectable **applied)
{
    if (applied) *applied=NULL;
    if (!adapter || !adapter->service || !adapter->diagnostics) return E_INVALIDARG;
    if (!adapter->connected || InterlockedCompareExchange(&adapter->stopping,0,0)) return S_FALSE;
    if (adapter->session.thread_id && adapter->session.thread_id!=GetCurrentThreadId()) return S_FALSE;
    if (mutation==Remove) {
        if (!element.Handle || !adapter->session.service) return S_FALSE;
        HRESULT hr=win16_xaml_button_set_remove_handle(&adapter->session.buttons,adapter->service,
                                                        adapter->diagnostics,element.Handle);
        if (hr==S_OK && !adapter->session.buttons.first)
            (void)InterlockedCompareExchange((volatile LONG *)&adapter->session.thread_id,0,(LONG)GetCurrentThreadId());
        return hr;
    }
    if (mutation!=Add) return S_FALSE;
    BOOL button=element.Type && (!wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.Button") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.CheckBox") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.RadioButton") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.TextBox") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.ComboBox") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.ComboBoxItem") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.Primitives.ScrollBar") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.ListBox") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.ListBoxItem") ||
                                 !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.ProgressBar") ||
                            !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.ToggleSwitch") ||
                            !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.Slider") ||
                            !wcscmp(element.Type,L"Microsoft.UI.Xaml.Controls.Primitives.ToggleButton"));
    BOOL root=!relation.Parent;
    InstanceHandle handle=(button || root) ? element.Handle : relation.Parent;
    if (!handle) return S_FALSE;
    IInspectable *object=NULL;
    HRESULT hr=IXamlDiagnostics_GetIInspectableFromHandle(adapter->diagnostics,handle,&object);
    if (InterlockedCompareExchange(&adapter->stopping,0,0)) {
        if (object) IInspectable_Release(object);
        return S_FALSE;
    }
    /* A visual may disappear between notification and handle resolution. */
    if (hr==HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) return S_FALSE;
    if (SUCCEEDED(hr) && !object) hr=E_FAIL;
    if (SUCCEEDED(hr)) {
        AcquireSRWLockExclusive(&adapter->init_lock);
        if (InterlockedCompareExchange(&adapter->stopping,0,0)) hr=S_FALSE;
        else if (!adapter->session.service)
            hr=win16_xaml_session_init(&adapter->session,adapter->service,adapter->diagnostics);
        ReleaseSRWLockExclusive(&adapter->init_lock);
    }
    if (hr==S_OK && root && scanning_root) hr=S_FALSE;
    else if (hr==S_OK && root) {
        scanning_root=TRUE;
        hr=win16_xaml_session_observe_root(&adapter->session,object,applied);
        scanning_root=FALSE;
    } else if (hr==S_OK) hr=win16_xaml_session_observe(&adapter->session,object,applied);
    if (object) IInspectable_Release(object);
    return hr;
}
HRESULT win16_xaml_adapter_stop(win16_xaml_adapter *adapter)
{
    if (!adapter || !adapter->service || !adapter->diagnostics) return E_INVALIDARG;
    if (adapter->session.thread_id && adapter->session.thread_id!=GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
    InterlockedExchange(&adapter->stopping,1);
    HRESULT hr=adapter->session.service ? win16_xaml_session_stop(&adapter->session) : S_OK;
    if (SUCCEEDED(hr) && !adapter->connected) release_interfaces(adapter);
    return hr;
}
HRESULT win16_xaml_adapter_remove_window(win16_xaml_adapter *adapter,HWND window)
{
    if (!adapter || !window) return E_INVALIDARG;
    if (!adapter->session.service) return S_FALSE;
    HRESULT hr=win16_xaml_button_set_remove_window(&adapter->session.buttons,adapter->service,adapter->diagnostics,window);
    if (SUCCEEDED(hr) && !adapter->session.buttons.first)
        (void)InterlockedCompareExchange((volatile LONG *)&adapter->session.thread_id,0,(LONG)GetCurrentThreadId());
    return hr;
}
void win16_xaml_adapter_disconnect(win16_xaml_adapter *adapter)
{
    if (!adapter) return;
    adapter->connected=FALSE;
    if (!adapter->session.service) release_interfaces(adapter);
}
