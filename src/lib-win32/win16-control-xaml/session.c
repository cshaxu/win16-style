#define COBJMACROS
#include "session.h"
#include "visual_tree.h"
#include <winstring.h>
#include <wchar.h>

typedef struct observation { win16_xaml_session *session; HRESULT result; IInspectable *applied; BOOL subtree; } observation;

static BOOL visit(IInspectable *object, unsigned int depth, void *context)
{
    (void)depth;
    observation *o=context;
    if (FAILED(o->result)) return FALSE;
    HSTRING name=NULL;
    HRESULT hr=IInspectable_GetRuntimeClassName(object,&name);
    if (FAILED(hr)) { o->result=hr; return FALSE; }
    PCWSTR runtime=name ? WindowsGetStringRawBuffer(name,NULL) : NULL;
    BOOL button=runtime && (!wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.Button") ||
                            !wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.CheckBox") ||
                            !wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.RadioButton") ||
                            !wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.TextBox") ||
                            !wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.ComboBox") ||
                            !wcscmp(runtime,L"Microsoft.UI.Xaml.Controls.ComboBoxItem"));
    if (name) WindowsDeleteString(name);
    if (!button) return TRUE;
    DWORD thread=GetCurrentThreadId();
    DWORD owner=(DWORD)InterlockedCompareExchange((volatile LONG *)&o->session->thread_id,(LONG)thread,0);
    if (owner && owner!=thread) { o->result=S_FALSE; return FALSE; }
    win16_xaml_button_style *state=NULL;
    hr=win16_xaml_button_set_add(&o->session->buttons,o->session->service,o->session->diagnostics,object,&state);
    /* An unsupported Button must not claim the only UI thread forever. */
    if (FAILED(hr) && !owner && !o->session->buttons.first)
        (void)InterlockedCompareExchange((volatile LONG *)&o->session->thread_id,0,(LONG)thread);
    if (hr==HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) || hr==HRESULT_FROM_WIN32(ERROR_BUSY)) hr=S_FALSE;
    if (hr==S_OK && !o->applied) { IInspectable_AddRef(object); o->applied=object; }
    if (FAILED(hr) || (SUCCEEDED(o->result) && o->result!=S_OK)) o->result=hr;
    return o->subtree && SUCCEEDED(hr);
}

HRESULT win16_xaml_session_init(win16_xaml_session *session, IVisualTreeService *service, IXamlDiagnostics *diagnostics)
{
    if (!session || !service || !diagnostics || session->service || session->diagnostics) return E_INVALIDARG;
    session->service=service; session->diagnostics=diagnostics;
    return S_OK;
}

HRESULT win16_xaml_session_observe(win16_xaml_session *session, IInspectable *object, IInspectable **applied)
{
    if (applied) *applied=NULL;
    if (!session || !session->service || !session->diagnostics || !object) return E_INVALIDARG;
    if (session->thread_id && session->thread_id!=GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
    if (session->stopping) return S_FALSE;
    observation o={session,S_FALSE,NULL,FALSE};
    HRESULT hr=win16_xaml_visit_ancestors(object,visit,&o);
    if (o.applied) {
        if (SUCCEEDED(hr) && applied) *applied=o.applied;
        else IInspectable_Release(o.applied);
    }
    return FAILED(hr) ? hr : o.result;
}

HRESULT win16_xaml_session_observe_root(win16_xaml_session *session,IInspectable *object,IInspectable **applied)
{
    if (applied) *applied=NULL;
    if (!session || !session->service || !session->diagnostics || !object) return E_INVALIDARG;
    if (session->thread_id && session->thread_id!=GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
    if (session->stopping) return S_FALSE;
    observation o={session,S_FALSE,NULL,TRUE};
    HRESULT hr=win16_xaml_visit_descendants(object,visit,&o);
    if (o.applied) {
        if (SUCCEEDED(hr) && applied) *applied=o.applied;
        else IInspectable_Release(o.applied);
    }
    return FAILED(hr) ? hr : o.result;
}

HRESULT win16_xaml_session_stop(win16_xaml_session *session)
{
    if (!session || !session->service || !session->diagnostics) return E_INVALIDARG;
    if (session->thread_id && session->thread_id!=GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
    session->stopping=TRUE;
    HRESULT hr=win16_xaml_button_set_remove_all(&session->buttons,session->service,session->diagnostics);
    if (SUCCEEDED(hr)) *session=(win16_xaml_session){0};
    return hr;
}
