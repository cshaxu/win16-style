#ifndef STYLE_CLASSIC_COM_CONTROLLER_H
#define STYLE_CLASSIC_COM_CONTROLLER_H
#include <windows.h>
/* One fixed protocol configuration per linked controller. Strings and callback
 * code remain alive for all hooks, factories, proxies and their callbacks.
 * The installer only binds the three existing COM entries through the caller's
 * transaction; it does not own a hook backend or add rollback/unload behavior. */
typedef struct style_classic_com_options {
    BOOL (*session_active)(void);
    HRESULT (*scope)(HRESULT (*)(void *),void *);
    PCWSTR property,folder_message,hidden_message;
    BOOL experimental_events,experimental_hidden;
} style_classic_com_options;
DWORD style_classic_com_attach(DWORD (*)(void *,void *),const style_classic_com_options *);
#endif
