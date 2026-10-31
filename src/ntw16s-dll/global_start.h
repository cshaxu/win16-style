#ifndef NTW16S_GLOBAL_START_H
#define NTW16S_GLOBAL_START_H
#include <windows.h>

/* Zero-initialize; retain the ticket across timeouts. The caller must retain
 * a verified target DLL reference until the worker returns. The session's
 * stop event controls the worker. close() never terminates target or thread. */
typedef struct ntw16s_global_start {
    HANDLE process,thread,ready;
    void *remote_packet;
} ntw16s_global_start;

DWORD ntw16s_global_start_begin(HANDLE process,DWORD pid,DWORD thread_id,DWORD cookie,
                                const WCHAR *dll_path,SECURITY_ATTRIBUTES *security,ntw16s_global_start *ticket);
DWORD ntw16s_global_start_ready(ntw16s_global_start *ticket,DWORD timeout);
DWORD ntw16s_global_start_close(ntw16s_global_start *ticket);
#endif
