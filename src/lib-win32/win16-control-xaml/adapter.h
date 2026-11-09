#ifndef WIN16_XAML_ADAPTER_H
#define WIN16_XAML_ADAPTER_H
#include "session.h"

/* Experimental single-UI-thread adapter. The owner must serialize disconnect
 * with callbacks, retain this object until callbacks stop, and call stop on
 * the owning UI thread before disposal. After disconnect, service references
 * remain retained until stop succeeds. Other UI threads are not yet handled. */
typedef struct win16_xaml_adapter {
    IVisualTreeService *service;
    IXamlDiagnostics *diagnostics;
    win16_xaml_session session;
    BOOL connected;
    LONG stopping;
    SRWLOCK init_lock;
} win16_xaml_adapter;

HRESULT win16_xaml_adapter_connect(win16_xaml_adapter *adapter,IVisualTreeService *service,IXamlDiagnostics *diagnostics);
HRESULT win16_xaml_adapter_change(win16_xaml_adapter *adapter,ParentChildRelation relation,
                                  VisualElement element,VisualMutationType mutation,IInspectable **applied);
HRESULT win16_xaml_adapter_stop(win16_xaml_adapter *adapter);
void win16_xaml_adapter_disconnect(win16_xaml_adapter *adapter);
#endif
