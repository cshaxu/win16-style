#ifndef STYLE_RUNTIME_PAINT_DISPATCH_H
#define STYLE_RUNTIME_PAINT_DISPATCH_H
#include "runtime_native.h"
#include "runtime_text.h"
#include "frame_paint.h"
#include "checklist.h"
#include "../../control/month.h"
#include "../../control/status_paint.h"
#include "../../control/native_overlay.h"
/* GUI-thread manager owns observation state only. Caller pins complete public
 * instances/runtime and all context, HWND/DC/module/font borrows. No renderer,
 * window attachment, process policy or hook is installed here. */
typedef struct style_runtime_paint_scope {
    struct style_runtime_paint_scope *previous;style_runtime_observation observation;
    style_control_month_state month;style_runtime_frame_scope frame;
} style_runtime_paint_scope;
typedef struct style_runtime_cool_scope {
    struct style_runtime_cool_scope *previous;style_runtime_observation observation;BOOL ours;
} style_runtime_cool_scope;
typedef struct style_runtime_box_scope {
    struct style_runtime_box_scope *previous;style_runtime_observation observation;BOOL witnessed;
} style_runtime_box_scope;
typedef struct style_runtime_bit_scope {
    struct style_runtime_bit_scope *previous;style_runtime_observation observation;HDC dc;RECT item;UINT state;
} style_runtime_bit_scope;
typedef struct style_runtime_paint_manager {
    style_runtime_paint_scope *paint;style_runtime_cool_scope *cool;style_runtime_box_scope *box;style_runtime_bit_scope *bit;
    HWND lookup_popup;style_runtime_checklists checklists;
    style_control_status_scope *status;style_control_status_themes status_themes;
    style_control_spin_scope *spin,*spin_stack;
    style_runtime_native_calls native_calls;style_control_month_queries month_queries;
} style_runtime_paint_manager;
typedef enum style_runtime_dispatch {
    STYLE_RUNTIME_DISPATCH_NATIVE,STYLE_RUNTIME_DISPATCH_COOL,STYLE_RUNTIME_DISPATCH_STATUS,
    STYLE_RUNTIME_DISPATCH_COLOR,STYLE_RUNTIME_DISPATCH_SPIN,STYLE_RUNTIME_DISPATCH_BOX,
    STYLE_RUNTIME_DISPATCH_LOOKUP,STYLE_RUNTIME_DISPATCH_STATIC,STYLE_RUNTIME_DISPATCH_BIT
} style_runtime_dispatch;
BOOL style_runtime_paint_eligible(const style_instance *,const style_control_runtime *,UINT);
style_runtime_dispatch style_runtime_dispatch_kind(HWND,const style_instance *,const style_control_runtime *,UINT,WPARAM,LPARAM,BOOL,BOOL,BOOL,BOOL);
typedef struct style_runtime_paint_cleanup {
    style_runtime_text_scope *text;style_control_status_scope *status_prior;style_control_spin_scope *spin_prior;
    style_control_spin_scope *spin;style_control_native_overlay *overlay;
    BOOL *box_witnessed;const DRAWITEMSTRUCT *item;
} style_runtime_paint_cleanup;
/* Caller has already retained one instance call. Normal return pops observation
 * state; caller completes its ordinary resource cleanup and release. Native
 * unwind restores each existing variant's resources and consumes that call
 * exactly once, even beneath a nested static dispatch. NATIVE is passthrough
 * with the preexisting absence of unwind cleanup. BIT requires item; SPIN
 * requires spin; COLOR requires overlay; variants other than STATIC/BIT/NATIVE
 * require text.
 * No exceptions are caught/translated, and no physical DLL unload is implied. */
LRESULT style_runtime_paint_dispatch(style_runtime_paint_manager *,const style_runtime_observation *,style_runtime_dispatch,HWND,UINT,WPARAM,LPARAM,const style_runtime_paint_cleanup *);
#endif
