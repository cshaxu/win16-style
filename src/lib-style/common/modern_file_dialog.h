#ifndef STYLE_MODERN_FILE_DIALOG_H
#define STYLE_MODERN_FILE_DIALOG_H
#include <windows.h>
typedef HRESULT (*style_dialog_scoped_call)(void *);
typedef struct style_dialog_modern_services {
    void (*enter)(void);
    void (*leave)(void);
    HRESULT (*scope)(style_dialog_scoped_call,void *);
} style_dialog_modern_services;
/* kind: 1=open, 2=save, 3=folder. Services and callbacks must outlive the call. */
HRESULT style_dialog_show_modern(HWND owner,UINT kind,PWSTR path,DWORD capacity,
    const style_dialog_modern_services *services);
#endif
