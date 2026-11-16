#ifndef STYLE_GLOBAL_WORKER_H
#define STYLE_GLOBAL_WORKER_H
#include "start.h"
typedef struct style_global_worker_ops {
    style_global_namespace *space;
    HOOKPROC hook;
    /* Optional complete process initialization. Presence preserves module
     * authentication and pinning before the callback; no physical unload. */
    DWORD (*initialize)(HMODULE module,DWORD cookie,void *context);
    void *context;
} style_global_worker_ops;
/* Runs outside DllMain on a target worker, verifies/copies the packet, joins
 * the selected GUI desktop and owns all temporary handles/maps/hooks until
 * stop, owner exit or GUI-thread exit. Ops/context outlive this call. */
DWORD style_global_worker_run(const style_global_worker_ops *,void *parameter);
#endif
