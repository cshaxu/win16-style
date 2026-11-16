#ifndef STYLE_GLOBAL_RECEIVER_H
#define STYLE_GLOBAL_RECEIVER_H
#include "late.h"
#include "lib-win32/process-global/policy.h"
typedef struct style_global_receiver_ops {
    style_global_namespace *space;
    style_global_late *late;
    PCWSTR control;
    LPARAM apply,remove;
    win32_process_global_image_filter allowed;
    BOOL (*eligible)(HWND);
    void (*change)(HWND,DWORD style,BOOL apply,void *);
    void *context;
} style_global_receiver_ops;
/* Zeroed process-lifetime state/config. change runs on the window GUI thread.
 * The component owns image-policy once state and thread reentry suppression;
 * it authenticates sessions, preserves LastError and always forwards the hook. */
typedef struct style_global_receiver {
    INIT_ONCE once;
    BOOL allowed;
    const style_global_receiver_ops *ops;
} style_global_receiver;
LRESULT style_global_receiver_handle(style_global_receiver *,int,WPARAM,LPARAM);
#endif
