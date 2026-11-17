#ifndef STYLE_CLASSIC_COM_HOOK_H
#define STYLE_CLASSIC_COM_HOOK_H
#include "classic_dialog_events.h"
/* The proxy and the immutable protocol configuration stay alive throughout
 * native initialization and every message callback. Navigation is one whole
 * caller operation, including its existing normal/experimental resource scope. */
typedef struct style_classic_com_hook_services {
    const style_classic_dialog_message_services *messages;
    PCWSTR navigation_message;
    style_classic_dialog_navigate_fn navigate;
} style_classic_com_hook_services;
UINT_PTR style_classic_com_hook(HWND,UINT,WPARAM,LPARAM,const style_classic_com_hook_services *);
#endif
