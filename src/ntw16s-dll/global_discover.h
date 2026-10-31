#ifndef NTW16S_GLOBAL_DISCOVER_H
#define NTW16S_GLOBAL_DISCOVER_H
#include <windows.h>

typedef void (*ntw16s_global_discovered)(HWND window,DWORD process,DWORD thread,void *context);
typedef struct ntw16s_global_discover {
    HWINEVENTHOOK hook;
    ntw16s_global_discovered callback;
    void *context;
    DWORD owner_thread;
    unsigned dispatching;
} ntw16s_global_discover;

/* Start, callbacks, and stop must run on one message-pumping thread. Window
 * identity can change immediately after the callback; consumers revalidate. */
DWORD ntw16s_global_discover_start(ntw16s_global_discover *watcher,ntw16s_global_discovered callback,void *context);
DWORD ntw16s_global_discover_stop(ntw16s_global_discover *watcher);
#endif
