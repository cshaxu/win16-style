#ifndef NTW16S_UWP_GLOBAL_H
#define NTW16S_UWP_GLOBAL_H
#include "lib-win32/process-global/policy.h"
typedef struct ntw16s_uwp_global ntw16s_uwp_global;
/* Owner-thread admission/stop; borrowed policy, DLL path and session handles
 * remain valid until stop succeeds. Failed stop retains the object for retry. */
DWORD ntw16s_uwp_global_start(const win32_process_global_policy *policy,PCWSTR dll,
    HDESK desktop,HANDLE quit,HANDLE stop,DWORD cookie,ntw16s_uwp_global **result);
BOOL ntw16s_uwp_global_observe(ntw16s_uwp_global *,HWND);
DWORD ntw16s_uwp_global_wait_idle(ntw16s_uwp_global *,DWORD timeout);
void ntw16s_uwp_global_stats_read(ntw16s_uwp_global *,LONG *queued,LONG *active,LONG *failed);
DWORD ntw16s_uwp_global_stop(ntw16s_uwp_global *,DWORD timeout);
#endif
