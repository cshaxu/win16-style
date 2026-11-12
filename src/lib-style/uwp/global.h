#ifndef STYLE_UWP_GLOBAL_H
#define STYLE_UWP_GLOBAL_H
#include "target.h"
#include "lib-win32/process-global/policy.h"
typedef struct style_uwp_global style_uwp_global;
typedef struct style_uwp_global_services {
    WORD machine;
    win32_process_global_image_filter host_image;
    BOOL (*active)(void *);
    DWORD (*change)(const style_uwp_target *,PCWSTR,BOOL,void *);
    void (*report)(const style_uwp_target *,DWORD,void *);
    void *context;
} style_uwp_global_services;
/* Owner-thread admission/stop. Policy, DLL path, handles and callback context
 * are borrowed until stop succeeds. Failed stop retains the object for retry.
 * Callbacks may run on the worker thread; services are copied on start. */
DWORD style_uwp_global_start(const win32_process_global_policy *,PCWSTR,HDESK,HANDLE,HANDLE,
    const style_uwp_global_services *,style_uwp_global **);
BOOL style_uwp_global_observe(style_uwp_global *,HWND);
DWORD style_uwp_global_wait_idle(style_uwp_global *,DWORD);
void style_uwp_global_stats_read(style_uwp_global *,LONG *,LONG *,LONG *);
DWORD style_uwp_global_stop(style_uwp_global *,DWORD);
#endif
