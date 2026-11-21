#ifndef STYLE_STOCK_FIND_H
#define STYLE_STOCK_FIND_H
#include <windows.h>
#include <dlgs.h>
#include "lib-win32-style/application/window/frame.h"
#include "metrics.h"
#include "stock_client.h"

/* The stock Find template on recent Windows leaves twelve empty dialog
 * units for its optional wrap control. Keep the native state and controls. */
static inline void style_find_compact(HWND dialog,const classic_dialog_metrics *metrics)
{
    if(!metrics->find_width)return;
    RECT client,modern={0,0,metrics->find_width,metrics->find_native_height},
        classic={0,0,metrics->find_width,metrics->find_classic_height};
    HWND wrap=GetDlgItem(dialog,chx3);
    if(!wrap || (GetWindowLongPtrW(wrap,GWL_STYLE)&WS_VISIBLE))return;
    style_restore_stock_dialog_client(dialog,metrics->find_width,metrics->find_native_height);
    if(!GetClientRect(dialog,&client) ||
       !MapDialogRect(dialog,&modern) || !MapDialogRect(dialog,&classic) ||
       client.right!=modern.right || client.bottom!=modern.bottom)return;

    style_client_screen(dialog,&client);
    client.bottom=client.top+classic.bottom;
    style_frame_changed_restore(dialog,&client,TRUE);
}
#endif
