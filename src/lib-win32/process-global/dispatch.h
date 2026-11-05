#ifndef WIN32_PROCESS_GLOBAL_DISPATCH_H
#define WIN32_PROCESS_GLOBAL_DISPATCH_H
#include <windows.h>
/* Caller owns handles/context until the dispatch thread has exited. */
typedef BOOL (*win32_process_global_dispatch_next)(void *context);
typedef struct win32_process_global_dispatch {
    HDESK desktop;HANDLE quit,stop,work,ready;DWORD error;
    win32_process_global_dispatch_next next;void *context;
} win32_process_global_dispatch;
DWORD WINAPI win32_process_global_dispatch_run(void *parameter);
/* Pool and borrowed handles must remain valid until wait succeeds. On any
 * start failure the partially started pool remains owned by the caller, who
 * must signal quit, wait, then release. Zero initialize before first use. */
typedef struct win32_process_global_dispatch_pool {
    unsigned count;
    HANDLE threads[MAXIMUM_WAIT_OBJECTS], ready[MAXIMUM_WAIT_OBJECTS];
    win32_process_global_dispatch contexts[MAXIMUM_WAIT_OBJECTS];
} win32_process_global_dispatch_pool;
DWORD win32_process_global_dispatch_pool_start(win32_process_global_dispatch_pool *pool,
    unsigned count, HDESK desktop, HANDLE quit, HANDLE stop, HANDLE work,
    win32_process_global_dispatch_next next, void *context, DWORD timeout);
/* Caller signals quit; timeout preserves all handles for a later wait. */
DWORD win32_process_global_dispatch_pool_wait(win32_process_global_dispatch_pool *pool, ULONGLONG deadline);
/* Only call after all started threads have exited. */
void win32_process_global_dispatch_pool_release(win32_process_global_dispatch_pool *pool);
#endif
