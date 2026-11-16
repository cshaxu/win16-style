#ifndef STYLE_GLOBAL_HOST_H
#define STYLE_GLOBAL_HOST_H
#include "manager.h"
enum style_global_host_notice {STYLE_GLOBAL_HOST_PENDING,STYLE_GLOBAL_HOST_OWNER_TRACE};
typedef struct style_global_host_spec {
    const style_global_manager_config *manager;
    HOOKPROC hook;
    void (*report)(enum style_global_host_notice,void *);
    void *context;
} style_global_host_spec;
/* Runs on a message-pumping desktop thread outside DllMain. Borrows spec;
 * owns session/owner/ready/stop handles, desktop hook and manager lifetime.
 * Returns the original shutdown/start error; does not terminate the host.
 * Failed manager stop transfers it through retained for same-thread retry;
 * do not unload its code/config before stop succeeds or the host exits. */
DWORD style_global_host_run(const style_global_host_spec *,style_global_manager **retained);
#endif
