#ifndef STYLE_CLASSIC_DIALOG_PROXY_DISPATCH_H
#define STYLE_CLASSIC_DIALOG_PROXY_DISPATCH_H
#include "lib-style/common/classic_dialog_proxy.h"
typedef struct style_classic_dialog_proxy_dispatch_services {
    HRESULT (*show)(style_classic_dialog_proxy *,HWND);
    HRESULT (*advise)(style_classic_dialog_proxy *,IFileDialogEvents *,DWORD *);
    HRESULT (*unadvise)(style_classic_dialog_proxy *,DWORD);
    HRESULT (*set_folder)(style_classic_dialog_proxy *,IShellItem *);
} style_classic_dialog_proxy_dispatch_services;
HRESULT style_classic_dialog_proxy_dispatch(style_classic_dialog_proxy *,style_dialog_v2_operation,ULONG_PTR,ULONG_PTR,
    const style_classic_dialog_proxy_dispatch_services *,BOOL);
#endif
