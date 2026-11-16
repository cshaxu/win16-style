#ifndef STYLE_STARTUP_PROPAGATE_H
#define STYLE_STARTUP_PROPAGATE_H
#include "context.h"
#include "lib-win32/process-hook/propagate.h"

/* The configuration and callback context remain valid for process lifetime.
 * The existing hook transaction owns rollback and trampoline retention. */
typedef struct style_startup_propagation {
    const style_startup_context *process;
    PCWSTR dependency;
    BOOL (*active)(void *);
    void *context;
} style_startup_propagation;
DWORD style_startup_propagate(DWORD (*)(void *,void *),const style_startup_propagation *);
void style_startup_propagation_discard(const style_startup_propagation *);
#endif
