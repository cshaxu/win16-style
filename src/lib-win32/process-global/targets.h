#ifndef WIN32_PROCESS_GLOBAL_TARGETS_H
#define WIN32_PROCESS_GLOBAL_TARGETS_H
#include <windows.h>
#include "queue.h"
#include "registry.h"
#define WIN32_PROCESS_GLOBAL_TARGET_BUCKETS 257u
#define WIN32_PROCESS_GLOBAL_TARGET_LIMIT 16384u
typedef enum win32_process_global_target_state {WIN32_TARGET_QUEUED,WIN32_TARGET_WORKING,WIN32_TARGET_ACTIVE,WIN32_TARGET_FAILED} win32_process_global_target_state;
typedef struct win32_process_global_target {
    win32_process_global_queue_node queue_node;
    DWORD pid,thread_id;HWND window;win32_process_global_target_state state;unsigned attempts;
    win32_process_global_registry_node registry_node;
} win32_process_global_target;
typedef struct win32_process_global_target_ops {
    BOOL (*expired)(win32_process_global_target *,void *);
    BOOL (*active_finished)(win32_process_global_target *,void *);
    BOOL (*failed_retryable)(win32_process_global_target *,void *);
    DWORD (*execute)(win32_process_global_target *,void *,BOOL *retained);
    void (*dispose)(win32_process_global_target *,void *);
} win32_process_global_target_ops;
typedef struct win32_process_global_targets {
    SRWLOCK lock,process_locks[64];
    win32_process_global_registry_node *buckets[WIN32_PROCESS_GLOBAL_TARGET_BUCKETS];
    win32_process_global_registry registry;win32_process_global_queue queue;unsigned count;
    volatile LONG queued,working,active,failed,overflow;
    HANDLE work,changed;SIZE_T target_size;win32_process_global_target_ops ops;void *context;
} win32_process_global_targets;
/* Zero-initialize before init. Caller owns event handles; callbacks run on discovery or worker threads.
 * Target allocations are owned by this table; caller supplies a size at least
 * sizeof(target) and embeds target as its first member. */
DWORD win32_process_global_targets_init(win32_process_global_targets *,HANDLE,HANDLE,SIZE_T,const win32_process_global_target_ops *,void *);
void win32_process_global_targets_submit(win32_process_global_targets *,DWORD,DWORD,HWND);
BOOL win32_process_global_targets_next(win32_process_global_targets *);
BOOL win32_process_global_targets_contains(win32_process_global_targets *,DWORD,DWORD);
BOOL win32_process_global_targets_idle(win32_process_global_targets *);
BOOL win32_process_global_targets_has_process(win32_process_global_targets *,DWORD);
typedef DWORD (*win32_process_global_target_close_fn)(win32_process_global_target *,void *,ULONGLONG);
/* After workers stop, close each target under a shared deadline. On error the
 * table remains allocated and the caller may retry the operation. */
DWORD win32_process_global_targets_close(win32_process_global_targets *,ULONGLONG,win32_process_global_target_close_fn,void *);
/* After workers stop and application cleanup succeeds, release all records. */
void win32_process_global_targets_dispose(win32_process_global_targets *);
#endif
