#ifndef STYLE_CLASSIC_COM_SHOW_H
#define STYLE_CLASSIC_COM_SHOW_H
#include "classic_dialog_proxy.h"
/* The proxy owns configuration and results. Services are borrowed only for
 * this synchronous call; native fallback uses the same styling scope. */
typedef struct style_classic_com_show_services {
    LPOFNHOOKPROC classic_hook;
    BOOL experimental_events,experimental_hidden;
    BOOL (*session_active)(void);
    HRESULT (*scope)(HRESULT (*)(void *),void *);
} style_classic_com_show_services;
HRESULT style_classic_com_show(style_classic_dialog_proxy *,HWND,
    const style_classic_com_show_services *);
#endif
