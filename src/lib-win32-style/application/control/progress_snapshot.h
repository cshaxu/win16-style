#ifndef STYLE_CONTROL_PROGRESS_SNAPSHOT_H
#define STYLE_CONTROL_PROGRESS_SNAPSHOT_H
#include "native.h"
#include <commctrl.h>
BOOL style_progress_bound_supported(HWND);
BOOL style_progress_native_supported(HWND);
typedef struct style_progress_snapshot {
    RECT client;
    PBRANGE range;
    int position;
    LRESULT state;
    COLORREF bar,background;
    unsigned epoch;
} style_progress_snapshot;
/* Plain native values, no ownership transfer or painting. The profile's
 * supported predicate is checked after synchronous queries, as before. */
BOOL style_progress_snapshot_read(HWND,const style_instance *,const style_control_native_state *,BOOL (*supported)(HWND),style_progress_snapshot *);
#endif
