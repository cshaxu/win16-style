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
#endif
