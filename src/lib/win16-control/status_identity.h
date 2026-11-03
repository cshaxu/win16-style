#ifndef WIN16_CONTROL_STATUS_IDENTITY_H
#define WIN16_CONTROL_STATUS_IDENTITY_H
#include <windows.h>
typedef struct win16_control_status_identity_view {
    const BOOL *candidate,*detached;const HMODULE *witness_owner;const HWND *parent;
} win16_control_status_identity_view;
BOOL win16_control_status_identity(HWND,const win16_control_status_identity_view *,HMODULE *);
#endif
