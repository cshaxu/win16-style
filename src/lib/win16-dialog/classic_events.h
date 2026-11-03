#ifndef WIN16_DIALOG_CLASSIC_EVENTS_H
#define WIN16_DIALOG_CLASSIC_EVENTS_H
#include "proxy.h"
typedef HRESULT (*win16_dialog_changing_fn)(win16_dialog_proxy *,IShellItem *);
typedef FDE_OVERWRITE_RESPONSE (*win16_dialog_overwrite_fn)(win16_dialog_proxy *,IShellItem *);
BOOL win16_dialog_list_changing(HWND,win16_dialog_proxy *,win16_dialog_changing_fn);
BOOL win16_dialog_typed_changing(HWND,win16_dialog_proxy *,win16_dialog_changing_fn);
BOOL win16_dialog_drive_changing(HWND,win16_dialog_proxy *,win16_dialog_changing_fn);
BOOL win16_dialog_overwrite_check(HWND,win16_dialog_proxy *,win16_dialog_overwrite_fn);
#endif
