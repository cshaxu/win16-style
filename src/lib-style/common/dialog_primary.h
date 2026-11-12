#ifndef STYLE_DIALOG_PRIMARY_H
#define STYLE_DIALOG_PRIMARY_H
#include <windows.h>
#include <shobjidl.h>
#include "lib-style/common/dialog_v2.h"
typedef struct style_dialog_primary {
    const void *vtable;
    void *owner;
    IUnknown *variant;
    BOOL *unsupported;
    const style_dialog_identity_services *identity;
    style_dialog_v2_call call;
} style_dialog_primary;
BOOL style_dialog_proxy_iid(REFIID iid,BOOL save);
void style_dialog_primary_init(style_dialog_primary *,void *,IUnknown *,BOOL *,BOOL,
    const style_dialog_identity_services *,style_dialog_v2_call);
#endif
