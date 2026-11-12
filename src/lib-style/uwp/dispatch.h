#ifndef STYLE_UWP_DISPATCH_H
#define STYLE_UWP_DISPATCH_H
#include "target.h"
typedef struct style_uwp_dispatch_spec {
    WORD machine;
    const char *hook_export;
    PCWSTR message,result_property;
    DWORD timeout;
    DWORD (*action)(const style_uwp_target *,BOOL,WPARAM *,void *);
    void *context;
} style_uwp_dispatch_spec;
/* Called outside the target UI thread. Target handles are observations; the
 * function revalidates association, host architecture and module identity.
 * The caller owns the protocol, action policy and spec for the entire call. */
DWORD style_uwp_dispatch_change(const style_uwp_target *,PCWSTR,BOOL,const style_uwp_dispatch_spec *);
#endif
