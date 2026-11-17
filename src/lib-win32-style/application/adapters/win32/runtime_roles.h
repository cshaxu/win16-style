#ifndef STYLE_RUNTIME_ROLES_H
#define STYLE_RUNTIME_ROLES_H
#include "../../lifecycle/instance.h"
typedef struct style_runtime_verification {BOOL candidate,verified;} style_runtime_verification;
typedef struct style_runtime_child_state {BOOL candidate,verified;HWND parent;HMODULE owner;} style_runtime_child_state;
typedef struct style_runtime_reference {
    BOOL candidate;HWND parent,target;HMODULE owner;
    style_instance *held;
} style_runtime_reference;
typedef struct style_runtime_references {style_runtime_reference popup,mdi;} style_runtime_references;
typedef struct style_runtime_roles {
    style_runtime_verification text,bit;
    style_runtime_child_state inplace,nav;
    style_runtime_references references;
} style_runtime_roles;
/* Initialize a fresh reference; retained instance belongs to the related
 * target. It is NOT released on style removal. At final root disposal a copy
 * transfers ownership out of the dying root, then release preserves popup,
 * MDI order. Callers pin the root during synchronous identity/remove calls. */
void style_runtime_reference_init(style_runtime_reference *,BOOL,HWND,HWND,HMODULE,style_instance *);
void style_runtime_references_release(style_runtime_references *);
BOOL style_runtime_reference_matches(HWND,const style_instance *,const style_runtime_reference *,BOOL,BOOL (*)(HWND));
/* Caller selects owner-draw notification policy; common guard validates the
 * recorded parent before native runtime roles. Caller pins the root. Failed
 * removal leaves ownership intact and later role checks still run. */
struct style_control_runtime;
void style_runtime_roles_guard(HWND,UINT,style_instance *,struct style_control_runtime *,BOOL,BOOL (*)(HWND),BOOL (*)(HWND));
#endif
