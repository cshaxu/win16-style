#ifndef STYLE_CONTROL_REPORT_H
#define STYLE_CONTROL_REPORT_H
#include "runtime.h"
typedef struct style_control_report_ops {
    void *context;
    SUBCLASSPROC notify;
    BOOL (*install)(HWND,BOOL,void *,const BOOL *);
    BOOL (*remove)(HWND);
} style_control_report_ops;
/* Native list/tree protocol and parent notification lifetime. The caller pins
 * the instance/root, provides only rendering/removal policy, and retains its
 * owner context until the parent subclass is removed. Module/parent borrowed. */
void style_control_report_bind(HWND,style_instance *,style_control_runtime *,SUBCLASSPROC,void *);
void style_control_report_settle(HWND,UINT,style_instance *,style_control_runtime *,const style_control_report_ops *);
void style_control_report_unbind(style_control_runtime *,SUBCLASSPROC,const void *);
#endif
