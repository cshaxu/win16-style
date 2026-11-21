#ifndef STYLE_STOCK_CLIENT_H
#define STYLE_STOCK_CLIENT_H
#include "lib-win32-style/application/window/frame.h"

/* The native dialog chose its outer rectangle before the classic frame
 * changed its insets. With no expected size, retain the native client
 * even when Windows selected a different stock layout. */
static inline void style_restore_stock_dialog_client(HWND dialog,LONG width,LONG height)
{
    RECT stock={0,0,width,height},outer,client;
    if((width && !MapDialogRect(dialog,&stock)) || !GetWindowRect(dialog,&outer))return;
    RECT native_outer=stock;
    UINT dpi=GetDpiForWindow(dialog);
    if(!AdjustWindowRectExForDpi(&native_outer,(DWORD)GetWindowLongPtrW(dialog,GWL_STYLE),
       FALSE,(DWORD)GetWindowLongPtrW(dialog,GWL_EXSTYLE),dpi?dpi:96))return;
    LONG extra_width=outer.right-outer.left-(native_outer.right-native_outer.left);
    LONG extra_height=outer.bottom-outer.top-(native_outer.bottom-native_outer.top);
    if(width){
        if(extra_width || extra_height)return;
    }else{
        if(extra_width<=0 || extra_height<=0)return;
        stock.right=extra_width;
        stock.bottom=extra_height;
    }

    style_client_screen(dialog,&client);
    if(client.right-client.left==stock.right && client.bottom-client.top==stock.bottom)return;
    client.right=client.left+stock.right;
    client.bottom=client.top+stock.bottom;
    style_restore_client(dialog,&client);
}
#endif
