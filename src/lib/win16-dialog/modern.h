#ifndef WIN16_DIALOG_MODERN_H
#define WIN16_DIALOG_MODERN_H
#include <windows.h>
typedef HRESULT (*win16_dialog_scoped_call)(void *);
typedef struct win16_dialog_modern_services {
    void (*enter)(void);
    void (*leave)(void);
    HRESULT (*scope)(win16_dialog_scoped_call,void *);
} win16_dialog_modern_services;
HRESULT win16_dialog_show_modern(HWND owner,UINT kind,PWSTR path,DWORD capacity,
    const win16_dialog_modern_services *services);
#endif
