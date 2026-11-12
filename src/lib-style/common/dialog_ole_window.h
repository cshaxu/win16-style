#ifndef STYLE_DIALOG_OLE_WINDOW_H
#define STYLE_DIALOG_OLE_WINDOW_H
#include <windows.h>
#include <oleidl.h>
#include "lib-style/common/dialog_customize.h"
typedef struct style_dialog_ole_window {
    IOleWindow iface;
    void *owner;
    IOleWindow *native;
    HWND *active_window;
    const style_dialog_identity_services *services;
} style_dialog_ole_window;
void style_dialog_ole_window_init(style_dialog_ole_window *,void *,IOleWindow *,HWND *,
    const style_dialog_identity_services *);
#endif
