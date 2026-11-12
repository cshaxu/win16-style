#ifndef WIN16_CONTROL_COOL_GRIP_IDENTITY_H
#define WIN16_CONTROL_COOL_GRIP_IDENTITY_H
#include <windows.h>
#include <commctrl.h>
BOOL win16_control_cool_grip_identity(HWND,void *,const NMCUSTOMDRAW *,WNDPROC,BOOL (*)(HWND,void *));
LRESULT win16_control_cool_notify(HWND,UINT,WPARAM,LPARAM,void *,HWND,BOOL *,BOOL *,BOOL (*)(HWND,void *),BOOL (*)(HWND,void *,const NMCUSTOMDRAW *));
#endif
