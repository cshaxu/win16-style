#ifndef STYLE_CLASSIC_COM_CONTROLLER_H
#define STYLE_CLASSIC_COM_CONTROLLER_H
#include <windows.h>
typedef enum style_classic_com_route {
    STYLE_COM_NATIVE,STYLE_COM_CLASSIC,STYLE_COM_EXPLORER
} style_classic_com_route;
/* One fixed protocol configuration per linked controller. Strings and callback
 * code remain alive for all hooks, factories, proxies and their callbacks.
 * The installer only binds the three existing COM entries through the caller's
 * transaction; it does not own a hook backend or add rollback/unload behavior. */
typedef struct style_classic_com_options {
    BOOL (*session_active)(void);
    HRESULT (*scope)(HRESULT (*)(void *),void *);
    PCWSTR property,folder_message,hidden_message;
    BOOL experimental_events,experimental_hidden;
    BOOL explorer_files;
    BOOL (*explorer_active)(void);
    HRESULT (*explorer_scope)(HRESULT (*)(void *),void *);
    style_classic_com_route (*route)(BOOL folder);
} style_classic_com_options;
DWORD style_classic_com_attach(DWORD (*)(void *,void *),const style_classic_com_options *);
#endif
