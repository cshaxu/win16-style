#ifndef STYLE_CONTROL_RUNTIME_H
#define STYLE_CONTROL_RUNTIME_H
#include "../lifecycle/instance.h"
#include "control_protocol.h"
#include "edge.h"
#include "notify.h"
#include "spin.h"
#include "control_status.h"
#include "control_native_combo.h"
#include "../adapters/win32/runtime_identity.h"
#include "../adapters/win32/runtime_roles.h"
/* Caller-selected native runtime families, not rendering eligibility. Common
 * initialization derives metadata/borrows from these complete selections. */
typedef struct style_control_runtime_candidates {
    BOOL static_candidate,scroll_candidate,edit_candidate,tab_candidate,vcl_report_candidate;
    BOOL track_candidate,color_candidate,bit_candidate,inplace_candidate,nav_candidate;
    BOOL group_frame_candidate,panel_frame_candidate,group_button_candidate,grid_candidate;
    BOOL vcl_toolbar_candidate,status_candidate,date_candidate;
} style_control_runtime_candidates;
typedef struct style_control_runtime_extras {
    BOOL cool_candidate,box_candidate,controlbar_candidate;
    BOOL rich_month,rich_hotkey,rich_popup,rich_unicode,rich_candidate;
    BOOL vb_user_edge,vb_data_edge,vb_mdi_child;
} style_control_runtime_extras;

void style_control_vcl_classify(HWND,DWORD,BOOL,HMODULE,style_control_runtime_candidates *);
void style_control_vcl_classify_extra(HWND,DWORD,BOOL,HMODULE,style_control_runtime_extras *);

typedef enum style_control_runtime_kind {
    STYLE_RUNTIME_GROUP,STYLE_RUNTIME_SCROLL,STYLE_RUNTIME_TAB,STYLE_RUNTIME_TRACK,STYLE_RUNTIME_TOOLBAR,STYLE_RUNTIME_EDIT
} style_control_runtime_kind;
typedef struct style_control_runtime_binding {
    BOOL candidate,attempted,installed;
    HWND parent;
    HMODULE owner,witness;
} style_control_runtime_binding;
typedef struct style_control_runtime_report {
    style_control_runtime_binding binding;
    BOOL tree_role,tree;
    HWND window,parent;
    HMODULE module;
} style_control_runtime_report;
typedef struct style_control_runtime_cool {BOOL candidate,verified;HMODULE owner;style_control_notify_state notify;} style_control_runtime_cool;
/* One root owns the live protocol; each existing runtime role owns its first
 * paint state and borrowed native identity. Eligibility is caller policy. */
typedef struct style_control_runtime {
    style_control_protocol *protocol;
    style_control_runtime_binding group,scroll,tab,track,toolbar,edit;
    BOOL link,combo,spinedit;
    DWORD edit_style,combo_type;
    HWND combo_parent;
    style_control_runtime_report report;
    style_control_edge_state grid,user,data;
    style_runtime_frame_state box,controlbar,group_frame,panel_frame;
    style_runtime_rich_state rich;
    style_control_native_combo_state color,date;
    style_control_status_state status;
    style_control_runtime_cool cool;
    style_runtime_roles roles;
    style_control_spin_state spin;
} style_control_runtime;
typedef struct style_control_runtime_ops {
    void *context;
    BOOL (*install)(HWND,style_control_runtime_kind,void *,const BOOL *);
    BOOL (*remove)(HWND,style_control_runtime_kind,const void *);
} style_control_runtime_ops;
/* Caller pins instance/runtime through all synchronous callbacks. The installed
 * protocol stays root-owned if installation reentrantly retires the instance;
 * root disposal then releases it. Failed bind never publishes a protocol. */
void style_control_runtime_settle(HWND,UINT,style_instance *,style_control_runtime *,style_control_runtime_kind,const style_control_runtime_ops *);
void style_control_runtime_dispose(style_control_runtime *);
BOOL style_control_runtime_combo_current(HWND,const style_control_runtime *);
#endif
