#ifndef STYLE_GLOBAL_OWNER_H
#define STYLE_GLOBAL_OWNER_H
#include "manager.h"
enum style_global_owner_notice_kind {STYLE_GLOBAL_OWNER_SWEEP,STYLE_GLOBAL_OWNER_HOOK_FALLBACK,
    STYLE_GLOBAL_OWNER_HOST_EXIT,STYLE_GLOBAL_OWNER_PENDING,STYLE_GLOBAL_OWNER_ACTIVE,STYLE_GLOBAL_OWNER_PEER_TRACE};
typedef struct style_global_owner_notice {
    enum style_global_owner_notice_kind kind;
    DWORD error;
    BOOL applying,deadline;
    unsigned attempted,failed;
} style_global_owner_notice;
typedef struct style_global_owner_spec {
    const style_global_manager_config *manager;
    DWORD (*paths)(DWORD style,WCHAR *native_path,WCHAR *peer_path,DWORD capacity,void *);
    const char *hook_export;
    PCWSTR host_export;
    void (*report)(const style_global_owner_notice *,void *);
    void *context;
} style_global_owner_spec;
/* Full dual-width ownership is the existing x64-controller/x86-peer path;
 * x86 builds exercise shared failure contracts, not an x86 controller.
 * One active owner per process, outside DllMain, on a message-pumping thread.
 * Owns session/events/security/module/hook/peer and manager lifetimes. Parent
 * is borrowed. paths provides complete product package selection/validation.
 * A failed stop transfers the manager via retained for same-thread retry;
 * code/config remain live until stop succeeds or the owner process exits. */
DWORD style_global_owner_run(const style_global_owner_spec *,HANDLE parent,DWORD style,style_global_manager **retained);
#endif
