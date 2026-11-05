#ifndef WIN32_PROCESS_GLOBAL_WINDOW_H
#define WIN32_PROCESS_GLOBAL_WINDOW_H
#include <windows.h>

typedef void (*win32_process_global_window_callback)(HWND window,DWORD process,DWORD thread,void *context);
typedef struct win32_process_global_window {
    HWINEVENTHOOK hook;
    win32_process_global_window_callback callback;
    void *context;
    DWORD owner_thread;
    unsigned dispatching;
} win32_process_global_window;

/* Start, callbacks, and stop must run on one message-pumping thread. Window
 * identity can change immediately after the callback; consumers revalidate. */
DWORD win32_process_global_window_start(win32_process_global_window *watcher,win32_process_global_window_callback callback,void *context);
DWORD win32_process_global_window_refresh(win32_process_global_window *watcher);
DWORD win32_process_global_window_stop(win32_process_global_window *watcher);
/* Traverse each top-level window and its descendants until callback stops or
 * the monotonic deadline expires. Thread id zero selects the current desktop.
 * Window identities can change during traversal; callers revalidate. Returns
 * FALSE with ERROR_CANCELLED on callback stop, ERROR_TIMEOUT on deadline,
 * or the native enumeration error. */
typedef BOOL (*win32_process_global_visit_window)(HWND,void *);
BOOL win32_process_global_window_walk(DWORD thread_id,ULONGLONG deadline,win32_process_global_visit_window,void *);
#endif
