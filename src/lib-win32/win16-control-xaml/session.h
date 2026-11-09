#ifndef WIN16_XAML_SESSION_H
#define WIN16_XAML_SESSION_H
#include "button_style.h"

/* One session per XAML UI thread, bound when the first Button is found.
 * Service pointers are borrowed; discovery must be stopped before release. */
typedef struct win16_xaml_session {
    IVisualTreeService *service;
    IXamlDiagnostics *diagnostics;
    win16_xaml_button_set buttons;
    DWORD thread_id;
    BOOL stopping;
} win16_xaml_session;

HRESULT win16_xaml_session_init(win16_xaml_session *session, IVisualTreeService *service, IXamlDiagnostics *diagnostics);
/* Scans this object and its ancestors. S_OK indicates a newly styled Button;
 * S_FALSE means no change, including an already styled or unsupported Button.
 * If requested, applied receives an owned reference on S_OK. */
HRESULT win16_xaml_session_observe(win16_xaml_session *session, IInspectable *object, IInspectable **applied);
HRESULT win16_xaml_session_observe_root(win16_xaml_session *session, IInspectable *object, IInspectable **applied);
/* Prevents new observations, then restores owned styles on the UI thread. */
HRESULT win16_xaml_session_stop(win16_xaml_session *session);
#endif
