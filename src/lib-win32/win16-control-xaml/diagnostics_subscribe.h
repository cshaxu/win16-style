#ifndef WIN16_XAML_DIAGNOSTICS_SUBSCRIBE_H
#define WIN16_XAML_DIAGNOSTICS_SUBSCRIBE_H
#include <windows.h>
#include <xamlom.h>

/* Called from a COM-initialized apartment. callback must support COM
 * marshaling. Completion runs on an MTA worker; its context must remain valid
 * until completion. S_OK means the worker started, not that Advise succeeded;
 * caller owns the returned thread handle and must keep this module loaded
 * until that thread exits. */
typedef void (*win16_xaml_subscribe_done)(void *context,HRESULT result);
HRESULT win16_xaml_subscribe_async(IVisualTreeService *service,IVisualTreeServiceCallback *callback,
                                   win16_xaml_subscribe_done done,void *context,HANDLE *worker);
#endif
