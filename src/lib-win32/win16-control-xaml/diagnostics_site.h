#ifndef WIN16_XAML_DIAGNOSTICS_SITE_H
#define WIN16_XAML_DIAGNOSTICS_SITE_H
#include <windows.h>
#include <ocidl.h>
#include <xamlom.h>

/* The caller's context must outlive the COM object. Callbacks receive borrowed
 * interfaces. The owner coordinates session teardown on each XAML UI thread. */
typedef struct win16_xaml_site_handlers {
    HRESULT (*connect)(void *context, IVisualTreeService *service, IXamlDiagnostics *diagnostics,
                       IObjectWithSite *site);
    void (*disconnect)(void *context);
    HRESULT (*change)(void *context, ParentChildRelation relation, VisualElement element, VisualMutationType mutation);
    void (*ready)(void *context);
    void (*failed)(void *context, HRESULT error);
} win16_xaml_site_handlers;

HRESULT win16_xaml_site_create(const win16_xaml_site_handlers *handlers, void *context,
                               REFIID iid, void **object);
/* Async site requires COM initialized at creation and keeps its module loaded
 * externally until the subscription worker finishes. A SetSite during the
 * pending transition returns RPC_E_CALL_REJECTED. */
HRESULT win16_xaml_site_create_async(const win16_xaml_site_handlers *handlers, void *context,
                                     REFIID iid, void **object);
#endif
