#ifndef WIN16_DIALOG_PRIMARY_H
#define WIN16_DIALOG_PRIMARY_H
#include <windows.h>
#include <shobjidl.h>
#include "v2.h"
typedef struct win16_dialog_primary {
    const void *vtable;
    void *owner;
    IUnknown *variant;
    BOOL *unsupported;
    const win16_dialog_customize_services *identity;
    win16_dialog_v2_call call;
} win16_dialog_primary;
BOOL win16_dialog_proxy_iid(REFIID iid,BOOL save);
void win16_dialog_primary_init(win16_dialog_primary *,void *,IUnknown *,BOOL *,BOOL,
    const win16_dialog_customize_services *,win16_dialog_v2_call);
#endif
