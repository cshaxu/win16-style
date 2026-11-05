#ifndef WIN32_PROCESS_GLOBAL_ENGINE_H
#define WIN32_PROCESS_GLOBAL_ENGINE_H
#include "targets.h"
#include "dispatch.h"
/* Caller zero-initializes the engine and owns the supplied handles, desktop,
 * callback context and target-specific resources. All lifecycle calls must
 * run on the owner thread; submission may run concurrently until discovery is
 * stopped. On a failed start, call stop before releasing borrowed resources. */
typedef struct win32_process_global_engine {
    win32_process_global_targets targets;
    win32_process_global_dispatch_pool pool;
    HANDLE quit,stop,changed;
    DWORD owner;
    BOOL initialized;
} win32_process_global_engine;
DWORD win32_process_global_engine_start(win32_process_global_engine *,HDESK,HANDLE,HANDLE,HANDLE,HANDLE,
    unsigned,SIZE_T,const win32_process_global_target_ops *,void *,DWORD);
void win32_process_global_engine_submit(win32_process_global_engine *,DWORD,DWORD,HWND);
BOOL win32_process_global_engine_contains(win32_process_global_engine *,DWORD,DWORD);
BOOL win32_process_global_engine_has_process(win32_process_global_engine *,DWORD);
typedef struct win32_process_global_engine_stats {LONG queued,active,failed,overflow;} win32_process_global_engine_stats;
void win32_process_global_engine_stats_read(win32_process_global_engine *,win32_process_global_engine_stats *);
DWORD win32_process_global_engine_wait_idle(win32_process_global_engine *,DWORD);
/* Stop discovery first; timeout or close failure retains state for retry. */
DWORD win32_process_global_engine_stop(win32_process_global_engine *,DWORD,win32_process_global_target_close_fn,void *);
#endif
