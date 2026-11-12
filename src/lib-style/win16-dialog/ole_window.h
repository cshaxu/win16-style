#ifndef WIN16_DIALOG_OLE_WINDOW_H
#define WIN16_DIALOG_OLE_WINDOW_H
#include <windows.h>
#include <oleidl.h>
#include "customize.h"
typedef struct win16_dialog_ole_window {
    IOleWindow iface;
    void *owner;
    IOleWindow *native;
    HWND *active_window;
    const win16_dialog_customize_services *services;
} win16_dialog_ole_window;
void win16_dialog_ole_window_init(win16_dialog_ole_window *,void *,IOleWindow *,HWND *,
    const win16_dialog_customize_services *);
#endif
