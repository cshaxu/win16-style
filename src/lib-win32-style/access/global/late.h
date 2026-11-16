#ifndef STYLE_GLOBAL_LATE_H
#define STYLE_GLOBAL_LATE_H
#include "session.h"
#include "lib-win32/process-global/background.h"
typedef struct style_global_late_ops {
    const style_global_namespace *space;
    DWORD (*initialize)(HMODULE module,DWORD cookie,void *context);
    PCWSTR control;
    LPARAM apply;
    void *context;
} style_global_late_ops;
/* Process-lifetime state/config, used only after session authentication.
 * Scheduling pins this module; it does not add physical unload support.
 * FALSE means pending, busy or failure, as in the existing background API. */
typedef struct style_global_late {
    win32_process_global_background background;
    const style_global_late_ops *ops;
} style_global_late;
BOOL style_global_late_schedule(style_global_late *,DWORD cookie);
void style_global_late_mark(style_global_late *,DWORD cookie);
#endif
