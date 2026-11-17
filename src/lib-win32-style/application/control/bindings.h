#ifndef STYLE_CONTROL_BINDINGS_H
#define STYLE_CONTROL_BINDINGS_H
#include "combo.h"
#include "scroll_controller.h"
#include "native.h"
#include "notify.h"
#include "updown.h"
#include "retire.h"
/* Owned by its surrounding GUI instance, not a borrowed view of profile
 * fields. Native snapshots/children/notifications and scroll resources keep
 * their existing individual contracts. Dispose after the last instance call. */
typedef struct style_control_bindings {
    style_control_combo combo;style_scroll_controller scroll;
    style_control_native_state tab,header,progress;
    style_control_notify_state track,toolbar;style_control_updown updown;
    BOOL list_theme_owned;
} style_control_bindings;
void style_control_bindings_init(style_control_bindings *);
void style_control_bindings_dispose(style_control_bindings *,const style_control_state *);
/* Borrowed complete native theme operation. The caller pins owner/instance
 * through capture and enable; bindings own a successfully captured lease.
 * Cancellation is checked at the original final settlement, not eagerly. */
typedef struct style_control_list_theme_ops {
    const void *owner;const BOOL *cancelled;
    BOOL (*retained)(HWND),(*capture)(HWND),(*enable)(HWND,const void *);
} style_control_list_theme_ops;
typedef struct style_control_setup_options {
    BOOL frame_bars,owner_draw,combo_passthrough;
    const style_control_combo_ops *combo;
    const style_control_combo_setup_ops *children;
    const style_control_list_theme_ops *list_theme;
} style_control_setup_options;
/* Caller pins instance/state/bindings through all native/provider callbacks.
 * Options describe existing protocol roles, never a concrete appearance.
 * Returns the original setup failure; detach wins with ERROR_CANCELLED. */
BOOL style_control_bindings_setup(HWND,style_instance *,style_control_state *,style_control_bindings *,const style_control_setup_options *);
typedef struct style_control_binding_ops {
    void *context;SUBCLASSPROC track_notify,toolbar_notify;
    style_control_state *(*lookup)(HWND);
    BOOL (*remove_list)(HWND),(*terminal_list)(HWND);
    void (*theme_remove)(HWND,const void *);
    BOOL (*theme_terminal)(HWND,const void *);
} style_control_binding_ops;
/* Called after subclass detach while the surrounding instance stays pinned.
 * Unbinds owned native resources in order. Native HWND/DC/font handles remain
 * borrowed. Terminal release skips restoration and reports child failure;
 * ordinary restoration remains best-effort, with the original callback order. */
style_control_retired style_control_bindings_release(HWND,style_instance *,style_control_state *,style_control_bindings *,BOOL,BOOL,const style_control_binding_ops *);
#endif
