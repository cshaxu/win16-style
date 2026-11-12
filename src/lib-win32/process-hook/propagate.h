#ifndef PROCESS_HOOK_PROPAGATE_H
#define PROCESS_HOOK_PROPAGATE_H
#include <windows.h>
#include "helper.h"
/* One process-lifetime registration. Configuration and callback context must
 * remain valid until process exit. The caller's attach transaction owns rollback
 * on partial installation. `active` can disable propagation without removing
 * installed trampolines. Callbacks return errors normally; they must not let
 * exceptions escape. Unwind restores reentry state, not child ownership.
 * `accept_*` is policy; `install` receives only the
 * newly created suspended child and captured, unpatched CreateProcessW. */
typedef struct process_hook_propagation_ops {
    BOOL (*active)(void *);
    BOOL (*accept_w)(LPCWSTR,LPWSTR,void *);
    BOOL (*accept_a)(LPCSTR,LPSTR,void *);
    DWORD (*install)(HANDLE,process_hook_create_w,void *);
    void *context;
} process_hook_propagation_ops;
DWORD process_hook_propagation_attach(DWORD (*attach)(void *,void *),const process_hook_propagation_ops *);
/* Includes registrations being prepared; callers must treat these as owned. */
BOOL process_hook_propagation_registered(void);
/* Only after the owner's hook transaction has been fully rolled back. */
void process_hook_propagation_discard(const process_hook_propagation_ops *);
/* Optional token-preserving W entry, after propagation_attach in the same
 * transaction. The caller supplies the original token; no elevation or token
 * acquisition is performed here. Same suspension, rollback and reentry rules. */
DWORD process_hook_propagation_attach_as_user_w(DWORD (*attach)(void *,void *));
#endif
