#ifndef WIN16_DIALOG_PROXY_DISPATCH_H
#define WIN16_DIALOG_PROXY_DISPATCH_H
#include "proxy.h"
typedef struct win16_dialog_proxy_dispatch_services {
    HRESULT (*show)(win16_dialog_proxy *,HWND);
    HRESULT (*advise)(win16_dialog_proxy *,IFileDialogEvents *,DWORD *);
    HRESULT (*unadvise)(win16_dialog_proxy *,DWORD);
    HRESULT (*set_folder)(win16_dialog_proxy *,IShellItem *);
} win16_dialog_proxy_dispatch_services;
HRESULT win16_dialog_proxy_dispatch(win16_dialog_proxy *,win16_dialog_v2_operation,ULONG_PTR,ULONG_PTR,
    const win16_dialog_proxy_dispatch_services *,BOOL);
#endif
