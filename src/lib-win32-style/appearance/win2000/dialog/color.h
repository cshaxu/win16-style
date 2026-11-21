#ifndef WIN2K_DIALOG_COLOR_H
#define WIN2K_DIALOG_COLOR_H
#include "lib-win32-style/application/dialog/classic_dialog_controls.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include "stock.h"

/* The stock compact picker saves its expanded outer rectangle before our
 * frame attaches. Expanding reuses that rectangle with different margins. */
static inline void win2k_color_restore_expanded_client(const CWPRETSTRUCT *message,const classic_dialog_metrics *metrics)
{
    enum { define_custom=719, add_custom=712, red_edit=706 };
    if(!metrics->color_width)return;
    HWND dialog=message->hwnd;
    if(message->message!=WM_COMMAND || LOWORD(message->wParam)!=define_custom ||
       HIWORD(message->wParam)!=BN_CLICKED)return;
    HWND expand=GetDlgItem(dialog,define_custom);
    if(!expand || (HWND)message->lParam!=expand || IsWindowEnabled(expand) ||
       !IsWindowEnabled(GetDlgItem(dialog,add_custom)) ||
       !IsWindowEnabled(GetDlgItem(dialog,red_edit)) || !style_classic_color_template(dialog))return;

    win2k_restore_stock_dialog_client(dialog,metrics->color_width,metrics->color_height);
}
#endif
