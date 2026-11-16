#ifndef STYLE_RUNTIME_PROCESS_H
#define STYLE_RUNTIME_PROCESS_H
#include "lib-win32-style/access/startup/context.h"
#include "lib-win32/process-hook/late_batch.h"

/* Zero-initialized, process-lifetime owner. Successful and sticky failed
 * batches retain their patch/trampoline storage; physical removal is absent. */
typedef struct style_runtime_process {
    SRWLOCK lock;
    process_hook_late_patch hooks[PROCESS_HOOK_LATE_PATCH_LIMIT];
    process_hook_late_patch *items[PROCESS_HOOK_LATE_PATCH_LIMIT];
    process_hook_late_batch batch;
} style_runtime_process;
typedef struct style_runtime_process_ops {
    PCWSTR module;
    DWORD (*prepare)(DWORD (*attach)(void *,void *),BOOL reuse,DWORD flags,void *context);
    void (*discard)(void *);
    void *context;
} style_runtime_process_ops;
/* Borrows a public startup context and immutable operation table, both valid
 * for process life; state/context/ops and both callbacks must be nonnull.
 * prepare registers the complete caller-selected API set,
 * uses attach synchronously, and returns normally. discard is called only
 * after a fully rolled-back preparation owned by this operation.
 * A cookie adopts an existing context; strict rejects incompatible flags.
 * Session liveness/activation and style operations remain caller policy. */
DWORD style_runtime_process_init(style_runtime_process *,style_startup_context *,
    const style_runtime_process_ops *,PCWSTR,DWORD,DWORD,BOOL);
#endif
