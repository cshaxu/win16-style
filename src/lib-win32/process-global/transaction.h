#ifndef WIN32_PROCESS_GLOBAL_TRANSACTION_H
#define WIN32_PROCESS_GLOBAL_TRANSACTION_H
#include <windows.h>
#define WIN32_PROCESS_GLOBAL_LOADING 1u
#define WIN32_PROCESS_GLOBAL_STARTING 2u
#define WIN32_PROCESS_GLOBAL_ACTIVE 3u
typedef DWORD (*win32_process_global_transaction_step)(void *context);
typedef struct win32_process_global_transaction_ops {
    win32_process_global_transaction_step load,validate,start,ready,apply;
} win32_process_global_transaction_ops;
/* The owner serializes access and retains resources on pending or failure. */
DWORD win32_process_global_transaction_advance(DWORD *phase,DWORD *control_error,BOOL apply,void *context,const win32_process_global_transaction_ops *ops);
/* Owner retains all handles and callback state throughout the call. Pending
 * returns one or two borrowed handles; stop/quit remain caller-owned. */
typedef struct win32_process_global_transaction_run_ops {
    BOOL (*active)(void *);
    DWORD (*begin)(void *);
    DWORD (*advance)(void *);
    unsigned (*pending)(void *,HANDLE [2]);
    BOOL (*retry)(void *,DWORD);
    DWORD (*apply)(void *);
} win32_process_global_transaction_run_ops;
DWORD win32_process_global_transaction_run(HANDLE quit,HANDLE stop,ULONGLONG deadline,ULONGLONG begin_deadline,
    BOOL retry_not_ready,const win32_process_global_transaction_run_ops *,void *);
#endif
