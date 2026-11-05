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
#endif
