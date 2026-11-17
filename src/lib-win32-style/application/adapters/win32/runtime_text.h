#ifndef STYLE_RUNTIME_TEXT_H
#define STYLE_RUNTIME_TEXT_H
#include <windows.h>
#include <commctrl.h>
/* Synchronous GUI-thread observations. Scope owns its two nested links and
 * copied metadata; context/windows/modules/DC/cancel are borrowed and pinned
 * by the caller. No font, renderer, hook or window attachment is owned here. */
typedef struct style_runtime_text_scope {
    struct style_runtime_text_scope *previous,*native_previous;
    HWND native_window;HDC native_dc;BOOL native_active;
    HWND window;void *context;BOOL active,report,label_only;
    HDC report_dc;HMODULE report_module;const BOOL *cancelled;
} style_runtime_text_scope;
typedef struct style_runtime_text_role {HWND window;BOOL label_only;} style_runtime_text_role;
typedef struct style_runtime_text_ops {
    void *context;
    BOOL (*include_text)(HWND,UINT);
    style_runtime_text_role (*role)(HWND,void *);
} style_runtime_text_ops;
/* Policy callbacks are borrowed only during begin. Native observation is
 * published first, text observation before role selection (including barrier
 * roles with NULL window). End each begun scope once, in LIFO order; use the
 * existing dispatch/unwind cleanup when forwarding native calls can throw. */
void style_runtime_text_begin(style_runtime_text_scope *,HWND,UINT,void *,const style_runtime_text_ops *);
void style_runtime_text_end(style_runtime_text_scope *);
const style_runtime_text_scope *style_runtime_text_current(void);
BOOL style_runtime_text_native_target(HDC,HWND *);
void style_runtime_text_report_enable(style_runtime_text_scope *,HWND,HMODULE,const BOOL *,BOOL (*)(HWND));
void style_runtime_text_report_bind(HWND,HDC);
BOOL style_runtime_text_report_active(void);
BOOL style_runtime_text_report_target(HDC,HWND *);
BOOL style_runtime_text_report_caller(HMODULE);
BOOL style_runtime_text_active(void);
BOOL style_runtime_text_direct_allowed(void);
/* Native runtime caller identity only; no font repertoire or rendering policy. */
BOOL style_runtime_text_caller(const void *);
BOOL style_runtime_text_label_caller(const void *);
struct style_instance;struct style_control_runtime;
/* Native report eligibility and binding; caller pins the complete instance and
 * runtime through synchronous native queries. File-owner paint stays native. */
void style_runtime_text_prepare_report(style_runtime_text_scope *,HWND,WPARAM,struct style_instance *,struct style_control_runtime *,BOOL,SUBCLASSPROC,void *,BOOL (*)(HWND));
#endif
