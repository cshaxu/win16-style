#ifndef WIN2K_DIALOG_STOCK_H
#define WIN2K_DIALOG_STOCK_H
#include "lib-win32-style/application/window/frame.h"

/* The native dialog chose an outer rectangle before the classic frame
 * changed its insets. Leave application-sized and custom layouts alone. */
static inline void win2k_restore_stock_dialog_client(HWND dialog,LONG width,LONG height)
{
    RECT stock={0,0,width,height},outer,client;
    if(!MapDialogRect(dialog,&stock) || !GetWindowRect(dialog,&outer))return;
    RECT native_outer=stock;
    UINT dpi=GetDpiForWindow(dialog);
    if(!AdjustWindowRectExForDpi(&native_outer,(DWORD)GetWindowLongPtrW(dialog,GWL_STYLE),
       FALSE,(DWORD)GetWindowLongPtrW(dialog,GWL_EXSTYLE),dpi?dpi:96) ||
       outer.right-outer.left!=native_outer.right-native_outer.left ||
       outer.bottom-outer.top!=native_outer.bottom-native_outer.top)return;

    style_client_screen(dialog,&client);
    if(client.right-client.left==stock.right && client.bottom-client.top==stock.bottom)return;
    client.right=client.left+stock.right;
    client.bottom=client.top+stock.bottom;
    style_restore_client(dialog,&client);
}
#endif
