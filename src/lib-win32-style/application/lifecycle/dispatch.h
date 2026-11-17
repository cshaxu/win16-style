#ifndef STYLE_DISPATCH_H
#define STYLE_DISPATCH_H
#include <windows.h>
/* Synchronous GUI-thread call. Contexts are borrowed through finish; caller
 * owns/pins them. NULL invoke forwards to DefSubclassProc on the current
 * subclass stack. Finish runs exactly once: TRUE after normal return, FALSE
 * during native unwind. It owns this call's resource cleanup and must not
 * throw. Native result/LastError survive normal finish. No exception is caught,
 * retried or translated, and no window/style/hook is installed here. */
typedef struct style_dispatch_ops {
    void *call_context,*finish_context;
    LRESULT (*invoke)(void *,HWND,UINT,WPARAM,LPARAM);
    void (*finish)(void *,BOOL);
} style_dispatch_ops;
LRESULT style_dispatch_message(HWND,UINT,WPARAM,LPARAM,const style_dispatch_ops *);
#endif
