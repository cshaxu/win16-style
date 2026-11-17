#ifndef STYLE_RUNTIME_MESSAGE_H
#define STYLE_RUNTIME_MESSAGE_H
#include "paint_dispatch.h"
#include "../../control/report.h"
/* Complete operations and their contexts are borrowed synchronously. Guard
 * selects existing style support and invalidates its font cache. Prepare owns
 * its font/text policy; common code owns the message's scopes/resources.
 * No private product state is decoded by this adapter. */
typedef struct style_runtime_message_ops {
    void (*guard)(const style_runtime_observation *,UINT);
    void (*prepare)(const style_runtime_observation *,UINT,WPARAM,style_runtime_text_scope *);
    style_runtime_frame_ops frames;
    style_control_runtime_ops controls;
    style_control_edge_ops edges;
    style_control_report_ops report;
    style_control_native_button_painter button;
    const WCHAR *status_class;DWORD month_rejection;
} style_runtime_message_ops;
/* Acquires and returns one existing instance call. Uses the canonical paint
 * dispatch's variant-specific native-unwind behavior; no new catch/recovery
 * is added to default forwarding, preparation or post-native callbacks. */
LRESULT style_runtime_message_dispatch(style_runtime_paint_manager *,const style_runtime_observation *,UINT,WPARAM,LPARAM,const style_runtime_message_ops *);
/* Initial support selection is copied; identities/fonts stay caller policy.
 * Runtime, manager and instance are complete borrowed public objects. The
 * notify callback/context must survive until the owned subclass is removed. */
typedef struct style_runtime_report_notify_ops {
    SUBCLASSPROC notify;
    BOOL file_owner,button_owner,checklist;
    BOOL (*file_identity)(HWND),(*button_identity)(HWND);
    void (*prepare_text)(style_runtime_text_scope *,HWND,void *);
} style_runtime_report_notify_ops;
/* Ordinary native forwarding retains its existing normal-only scope cleanup.
 * BIT uses the canonical paint dispatch/unwind; no other recovery is added. */
LRESULT style_runtime_report_notify(style_runtime_paint_manager *,const style_runtime_observation *,HWND,UINT,WPARAM,LPARAM,UINT_PTR,const style_runtime_report_notify_ops *);
#endif
