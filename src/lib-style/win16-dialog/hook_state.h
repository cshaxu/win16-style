#ifndef WIN16_DIALOG_HOOK_STATE_H
#define WIN16_DIALOG_HOOK_STATE_H
#include "proxy.h"
void win16_dialog_hook_attach_base(win16_dialog_proxy *,HWND,BOOL);
PCWSTR win16_dialog_hook_label(void *,int);
void win16_dialog_hook_detach(void *,HWND,BOOL);
#endif
