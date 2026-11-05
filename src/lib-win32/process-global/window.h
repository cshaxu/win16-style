#ifndef WIN32_PROCESS_GLOBAL_WINDOW_H
#define WIN32_PROCESS_GLOBAL_WINDOW_H
#include <windows.h>

typedef void (*win32_process_global_window_callback)(HWND window,DWORD process,DWORD thread,void *context);
typedef struct win32_process_global_window {
    HWINEVENTHOOK hook;
    win32_process_global_window_callback callback;
    void *context;
    DWORD owner_thread;
    unsigned dispatching;DWORD flags;
} win32_process_global_window;

/* Start, callbacks, and stop must run on one message-pumping thread. Window
 * identity can change immediately after the callback; consumers revalidate. */
#define WIN32_PROCESS_GLOBAL_WINDOW_INCLUDE_SELF 0x00000001u
/* The legacy start excludes the caller process. start_ex lets callers opt in;
 * this is discovery only and does not authorize attaching to any target. */
DWORD win32_process_global_window_start_ex(win32_process_global_window *watcher,win32_process_global_window_callback callback,void *context,DWORD flags);
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
/* Installs one thread-specific hook, signals ready, waits for any borrowed
 * handle and unhooks before returning; no caller handle is closed. The hook
 * procedure and module are caller policy and must outlive the call. */
DWORD win32_process_global_window_hook_wait(int,HOOKPROC,HINSTANCE,DWORD,HANDLE,const HANDLE *,DWORD);
#endif
