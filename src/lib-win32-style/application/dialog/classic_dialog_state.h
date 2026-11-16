#ifndef STYLE_CLASSIC_DIALOG_STATE_H
#define STYLE_CLASSIC_DIALOG_STATE_H
#include "lib-win32-style/application/dialog/classic_dialog_proxy.h"
void style_classic_dialog_hook_attach_base(style_classic_dialog_proxy *,HWND,BOOL);
PCWSTR style_classic_dialog_hook_label(void *,int);
void style_classic_dialog_hook_detach(void *,HWND,BOOL);
#endif
