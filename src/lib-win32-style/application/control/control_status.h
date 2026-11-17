#ifndef STYLE_CONTROL_STATUS_H
#define STYLE_CONTROL_STATUS_H
#include "../lifecycle/instance.h"
/* Alias spelling is caller policy. Parent/module handles are borrowed; the
 * caller pins the complete instance/state throughout synchronous queries. */
typedef struct style_control_status_state {
    BOOL candidate,attempted,installed,present;
    HMODULE witness_owner;
    HWND parent;
    unsigned epoch;
} style_control_status_state;
BOOL style_control_status_identity(HWND,const style_instance *,const style_control_status_state *,LPCWSTR,HMODULE *);
BOOL style_control_status_settle(HWND,UINT,const style_instance *,style_control_status_state *,LPCWSTR);
void style_control_status_invalidate(UINT,BOOL *,style_control_status_state *);
/* Owns value buffers, no caller field pointers. Native module is borrowed.
 * Read initializes every member; invalidation preserves the previous protocol's
 * partial-pane behavior, with epoch/instance checked again before rendering. */
typedef struct style_control_status_snapshot {
    HMODULE native;
    unsigned epoch,count,text_count;
    BOOL simple;
    int simple_length;
    WCHAR simple_text[513];
    RECT panes[64],text_panes[64];
} style_control_status_snapshot;
void style_control_status_read(HWND,const style_instance *,const style_control_status_state *,LPCWSTR,style_control_status_snapshot *);
#endif
