#ifndef WIN2K_DIALOG_FIND_H
#define WIN2K_DIALOG_FIND_H
#include <windows.h>
#include <dlgs.h>
#include "lib-win32-style/application/window/frame.h"

/* The stock Find template on recent Windows leaves twelve empty dialog
 * units for its optional wrap control. Keep the native state and controls. */
static inline void win2k_find_compact(HWND dialog)
{
    RECT client,modern={0,0,236,74},classic={0,0,236,62};
    HWND wrap=GetDlgItem(dialog,chx3);
    if(!wrap || (GetWindowLongPtrW(wrap,GWL_STYLE)&WS_VISIBLE) ||
       !GetClientRect(dialog,&client) ||
       !MapDialogRect(dialog,&modern) || !MapDialogRect(dialog,&classic) ||
       client.right!=modern.right || client.bottom!=modern.bottom)return;

    style_client_screen(dialog,&client);
    client.bottom=client.top+classic.bottom;
    style_frame_changed_restore(dialog,&client,TRUE);
}
#endif
