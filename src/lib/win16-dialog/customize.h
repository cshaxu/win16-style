#ifndef WIN16_DIALOG_CUSTOMIZE_H
#define WIN16_DIALOG_CUSTOMIZE_H
#include <windows.h>
#include <shobjidl.h>
typedef struct win16_dialog_customize_services {
    HRESULT (*query)(void *,REFIID,void **);
    ULONG (*add)(void *);
    ULONG (*release)(void *);
} win16_dialog_customize_services;
typedef struct win16_dialog_customize {
    IFileDialogCustomize iface;
    void *owner;
    IFileDialogCustomize *native;
    BOOL *unsupported;
    const win16_dialog_customize_services *services;
} win16_dialog_customize;
void win16_dialog_customize_init(win16_dialog_customize *,void *,IFileDialogCustomize *,BOOL *,
    const win16_dialog_customize_services *);
#endif
