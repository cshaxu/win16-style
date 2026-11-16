#ifndef STYLE_GLOBAL_CONTROL_H
#define STYLE_GLOBAL_CONTROL_H
#include "session.h"
typedef struct style_global_control_ops {
    DWORD (*run)(HANDLE parent,DWORD style,void *context);
    void (*failed)(DWORD error,void *context);
    DWORD failed_result;
} style_global_control_ops;
/* GUI-owner-thread state. Parent/config/operations/context are borrowed until
 * close succeeds; the component owns worker. Renderer IDs and none are caller
 * values. A changed selection always stops the old worker before starting a
 * new one. The caller continues its own UI/message loop. */
typedef struct style_global_control {
    HANDLE parent,worker;
    BOOL enabled,busy,stopping;
    DWORD style,desired,none;
    const style_global_namespace *space;
    const style_global_control_ops *ops;
    void *context;
    WCHAR session_name[STYLE_GLOBAL_NAME_CAP],request_name[STYLE_GLOBAL_NAME_CAP+16];
} style_global_control;
BOOL style_global_control_init(style_global_control *,HANDLE,DWORD,DWORD,
    const style_global_namespace *,const style_global_control_ops *,void *);
void style_global_control_select(style_global_control *,DWORD);
/* Call only after worker signals. Closes it. TRUE means the controller should
 * terminate with *result; FALSE means an orderly switch/stop was reconciled. */
BOOL style_global_control_finished(style_global_control *,DWORD *result);
void style_global_control_update(style_global_control *);
/* Failed close retains worker and state for retry; never terminates a worker
 * or target. Parent ownership is unchanged. */
DWORD style_global_control_close(style_global_control *,DWORD timeout);
#endif
