/* Test-only: runs on the owning UI thread of a self-created target. */
#define COBJMACROS
#define INITGUID
#include "lib-win32/win16-control-xaml/adapter.h"
#include <stdint.h>

typedef struct observed_entry {
    void *next;
    IUnknown *identity;
    BOOL installing;
    win16_xaml_button_style style;
} observed_entry;
static IVisualTreeService *service;
static IXamlDiagnostics *diagnostics;
static IInspectable *button;
static void clear(void)
{
    if (button) IInspectable_Release(button);
    if (diagnostics) IXamlDiagnostics_Release(diagnostics);
    if (service) IVisualTreeService_Release(service);
    button=NULL; diagnostics=NULL; service=NULL;
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
        } else if (m->wParam==2 && button) {
            hr=win16_xaml_button_source(service,diagnostics,button,&source,&index);
            clear();
        }
        uintptr_t value=SUCCEEDED(hr) ? (uintptr_t)source+1 : ((uintptr_t)(DWORD)hr<<1)|1;
        SetPropW(m->hwnd,L"Win16Xaml.Test.Property.Result",(HANDLE)value);
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
