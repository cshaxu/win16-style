#ifndef STYLE_DIALOG_CUSTOMIZE_H
#define STYLE_DIALOG_CUSTOMIZE_H
#include <windows.h>
#include <shobjidl.h>
/* Identity, native interfaces, active state and fallback flag are borrowed.
 * These adapters implement classic-dialog compatibility: operations that
 * cannot be mirrored set the caller-owned unsupported flag. */
typedef struct style_dialog_identity_services {
    HRESULT (*query)(void *,REFIID,void **);
    ULONG (*add)(void *);
    ULONG (*release)(void *);
} style_dialog_identity_services;
typedef struct style_dialog_customize {
    IFileDialogCustomize iface;
    void *owner;
    IFileDialogCustomize *native;
    BOOL *unsupported;
    const style_dialog_identity_services *services;
} style_dialog_customize;
void style_dialog_customize_init(style_dialog_customize *,void *,IFileDialogCustomize *,BOOL *,
    const style_dialog_identity_services *);
#endif
