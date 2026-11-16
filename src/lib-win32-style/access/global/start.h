#ifndef STYLE_GLOBAL_START_H
#define STYLE_GLOBAL_START_H
#include "session.h"
#include "lib-win32/process-global/worker.h"
/* Immutable same-bitness packet, copied by the worker before ready signals.
 * Layout/version preserve the existing target-worker protocol. */
typedef struct style_global_thread_packet {
    DWORD bytes,version,thread_id,cookie;
} style_global_thread_packet;
C_ASSERT(sizeof(style_global_thread_packet)==16);
typedef struct style_global_start_config {
    const style_global_namespace *space;
    const char *worker_export;
} style_global_start_config;
/* Config, process and path are borrowed for this call. A zeroed ticket owns
 * the ready event, packet allocation and retained handles on return. Close
 * with win32_process_global_worker_close; busy/timeout retains it for retry.
 * Uses the existing same-bitness worker; no physical module unload. */
DWORD style_global_start_begin(const style_global_start_config *,HANDLE process,
    DWORD pid,DWORD thread_id,DWORD cookie,const WCHAR *dll_path,
    SECURITY_ATTRIBUTES *security,win32_process_global_worker *ticket);
#endif
