#ifndef WIN2K_DIALOG_COLOR_H
#define WIN2K_DIALOG_COLOR_H
#include "lib-win32-style/application/dialog/classic_dialog_controls.h"
#include "lib-win32-style/application/window/frame.h"

/* The stock compact picker saves its expanded outer rectangle before our
 * frame attaches. Expanding reuses that rectangle with different margins. */
static inline void win2k_color_restore_expanded_client(const CWPRETSTRUCT *message)
{
    enum { define_custom=719, add_custom=712, red_edit=706 };
    HWND dialog=message->hwnd;
    if(message->message!=WM_COMMAND || LOWORD(message->wParam)!=define_custom ||
       HIWORD(message->wParam)!=BN_CLICKED)return;
    HWND expand=GetDlgItem(dialog,define_custom);
    if(!expand || (HWND)message->lParam!=expand || IsWindowEnabled(expand) ||
       !IsWindowEnabled(GetDlgItem(dialog,add_custom)) ||
       !IsWindowEnabled(GetDlgItem(dialog,red_edit)) || !style_classic_color_template(dialog))return;

    RECT stock={0,0,298,184},outer,client;
    if(!MapDialogRect(dialog,&stock) || !GetWindowRect(dialog,&outer))return;
    RECT old_outer=stock;
    UINT dpi=GetDpiForWindow(dialog);
    if(!AdjustWindowRectExForDpi(&old_outer,(DWORD)GetWindowLongPtrW(dialog,GWL_STYLE),
       FALSE,(DWORD)GetWindowLongPtrW(dialog,GWL_EXSTYLE),dpi?dpi:96) ||
       outer.right-outer.left!=old_outer.right-old_outer.left ||
       outer.bottom-outer.top!=old_outer.bottom-old_outer.top)return;

    style_client_screen(dialog,&client);
    if(client.right-client.left==stock.right && client.bottom-client.top==stock.bottom)return;
    client.right=client.left+stock.right;
    client.bottom=client.top+stock.bottom;
    style_restore_client(dialog,&client);
}
#endif
