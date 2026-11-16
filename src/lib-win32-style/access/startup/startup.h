#ifndef WIN32_STYLE_STARTUP_H
#define WIN32_STYLE_STARTUP_H
#include "lib-win32/process-hook/startup_hooks.h"
#include "lib-win32/process-hook/staging.h"
#include "context.h"

/* The callback prepares the caller's own hooks in the active transaction.
 * Abort only covers preparation; commit retains its existing error contract. */
static inline DWORD win32_style_startup(DWORD (*prepare)(void *),void *context)
{
    DWORD error=process_hook_startup_begin();
    if(error)return error;
    error=prepare(context);
    if(error){process_hook_startup_abort();return error;}
    return process_hook_startup_commit();
}
/* Passive loads do not restore or initialize hooks. The caller chooses the
 * compatible module identity, session policy and complete hook preparation. */
DWORD style_startup_bootstrap(style_startup_context *,PCWSTR,BOOL,BOOL *,DWORD (*)(void *),void *);
#endif
