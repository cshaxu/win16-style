#ifndef WIN32_STYLE_STARTUP_H
#define WIN32_STYLE_STARTUP_H
#include "lib-win32/process-hook/startup_hooks.h"

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
#endif
