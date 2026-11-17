#ifndef STYLE_RUNTIME_ATTACHMENT_H
#define STYLE_RUNTIME_ATTACHMENT_H
#include "../../control/runtime.h"
/* Embed first in the caller's allocation. Instance must remain first so its
 * registered subclass data identifies this exact public root. Native protocol
 * and themed-target state belong here; rendering resources remain caller-owned.
 * instance.context/dispose still identify and finally free the complete owner. */
typedef struct style_runtime_attachment {
    style_instance instance;style_control_runtime runtime;HWND themed;
} style_runtime_attachment;
typedef struct style_runtime_owner_roles {BOOL file_owner,button_owner,checklist;} style_runtime_owner_roles;
typedef struct style_runtime_attachment_seed {
    const style_control_runtime_candidates *candidates;const style_control_runtime_extras *extra;
    HMODULE owner;style_control_protocol *protocol;
    BOOL report,tree,status,spin,vcl_mdi_probe,lookup_popup_candidate;
    HWND popup_source,popup_source_parent;style_instance *popup_source_instance;
    SUBCLASSPROC procedure;
    style_runtime_owner_roles (*owner_roles)(HWND,void *);void *context;
} style_runtime_attachment_seed;
/* Fresh initialized, unpublished root only. Adds the existing initial call and
 * related-instance borrows; protocol/state ownership is transferred to root.
 * Owner policy runs at its original phase, after initial identity/references.
 * Root and borrows are caller-owned until publication; publish settles failure. */
void style_runtime_attachment_initialize(HWND,DWORD,style_runtime_attachment *,const style_runtime_attachment_seed *);
/* Publish a fresh initialized root. Failure consumes its protocol, related
 * borrows and containing owner, in that order, preserving the native error.
 * discard frees only the unpublished owner; ordinary dispose is not used.
 * Success leaves the initial call retained for installation settlement. */
BOOL style_runtime_attachment_publish(HWND,style_runtime_attachment *,SUBCLASSPROC,void (*discard)(void *));
/* Finishes the provider's already-selected installation on a retained root.
 * Does not consume that reference. Owns native theme-target/scrollbar setup and
 * cancellation/rollback, reporting the original installation error separately.
 * The provider owns drawing and removal; no new unwind or unload is implied. */
typedef struct style_runtime_attachment_install_ops {
    BOOL (*get_combo_info)(HWND,PCOMBOBOXINFO);
    BOOL (*theme_enable)(HWND,const void *),(*list_scrollbar_enable)(HWND),(*edit_scrollbar_enable)(HWND);
    void (*theme_remove)(HWND,const void *);
    BOOL (*remove)(HWND);
} style_runtime_attachment_install_ops;
BOOL style_runtime_attachment_install_finish(HWND,style_runtime_attachment *,DWORD,BOOL,BOOL,BOOL,BOOL,DWORD *,const style_runtime_attachment_install_ops *);

/* Borrowed for one synchronous runtime message. Common code owns native
 * multiline-edit theme/scrollbar settlement; provider owns the actual renderer.
 * The root's existing instance.context is the renderer/theme owner. */
typedef struct style_runtime_attachment_controls {
    style_runtime_attachment *root;
    BOOL (*install)(HWND,style_control_runtime_kind,void *,const BOOL *);
    BOOL (*remove)(HWND,const void *);
    BOOL (*theme_enable)(HWND,const void *),(*edit_scrollbars)(HWND);
    void (*theme_remove)(HWND,const void *);
} style_runtime_attachment_controls;
/* Exact style_control_runtime_ops callbacks; context points to the above
 * complete operation. No reference is consumed or new unwind handling added.
 * Removal keeps the native renderer result even if its theme is cleared. */
BOOL style_runtime_attachment_control_install(HWND,style_control_runtime_kind,void *,const BOOL *);
BOOL style_runtime_attachment_control_remove(HWND,style_control_runtime_kind,const void *);

/* Complete runtime identity check for already selected roles. Unwitnessed
 * candidates remain current; this is not the public "fully attached" query.
 * Policy callbacks run at their original ordered phases, including no-root
 * targets. Each phase resolves the current root again; callbacks may replace it.
 * Uses native query errors as-is, without adding unwind or lifetime ownership. */
typedef struct style_runtime_attachment_policy {
    BOOL (*data_current)(HWND),(*user_current)(HWND);
    DWORD month_rejection;
} style_runtime_attachment_policy;
BOOL style_runtime_attachment_current(HWND,SUBCLASSPROC,const style_runtime_attachment_policy *);

typedef struct style_runtime_attachment_ops {
    BOOL (*cool_identity)(HWND,void *);
    BOOL (*host_attached)(HWND),(*user_label)(HWND);
    SUBCLASSPROC report_notify,cool_notify;
    BOOL (*frame_remove)(HWND,const void *),(*control_remove)(HWND,const void *);
    void (*theme_remove)(HWND,const void *);
    BOOL (*frame_terminal)(HWND),(*control_terminal)(HWND),(*theme_terminal)(HWND,const void *);
    void (*scrollbars)(HWND);
    DWORD month_rejection;
} style_runtime_attachment_ops;
/* Only use a procedure whose subclass data is this public root. Operations and
 * owner context stay alive through all callbacks. Normal removal preserves
 * failed-detach ownership; terminal visits exact nodes and reports first error,
 * without restoration or any physical DLL unload. */
BOOL style_runtime_attachment_attached(HWND,SUBCLASSPROC,const style_runtime_attachment_ops *);
void style_runtime_attachment_scroll_changed(HWND,SUBCLASSPROC,const style_runtime_attachment_ops *);
BOOL style_runtime_attachment_remove(HWND,SUBCLASSPROC,const style_runtime_attachment_ops *);
BOOL style_runtime_attachment_terminal(HWND,SUBCLASSPROC,const style_runtime_attachment_ops *);
#endif
