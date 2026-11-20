/* The original trackbar.c draws a sunken channel and a soft raised thumb.
 * Let the native notification supply client geometry and interaction state. */
#include "trackbar_paint.h"
#include "../common/paint.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

BOOL win2k_trackbar_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    DWORD extended=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    DWORD allowed=TBS_AUTOTICKS|TBS_VERT|TBS_TOP|TBS_BOTH|TBS_NOTICKS|TBS_FIXEDLENGTH|TBS_NOTHUMB;
    return !(style&(0xffffu&~allowed)) && !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(extended&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

BOOL win2k_trackbar_part(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    BOOL channel=draw->dwItemSpec==TBCD_CHANNEL;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    BOOL thumb=draw->dwItemSpec==TBCD_THUMB && !(style&TBS_NOTHUMB);
    RECT r=draw->rc,client;
    HDC dc=draw->hdc;
    if(!dc || (!channel && !thumb) || !GetClientRect(window,&client) ||
       IsRectEmpty(&r) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom)return FALSE;
    const win2k_trackbar_presentation *presentation=context;
    const classic_trackbar_colors *colors=&presentation->axes[(style&TBS_VERT)!=0];
    win2k_palette palette=win2k_standard_palette;
    if(colors->present){
        palette.face=colors->thumb;
        palette.dark=colors->edge;
    }
    style_trackbar_profile profile=presentation->profile?presentation->profile:STYLE_TRACKBAR_RELIEF;
    return style_trackbar_paint(dc,r,style,GetDpiForWindow(window),channel,
        (draw->uItemState&CDIS_SELECTED)!=0,!IsWindowEnabled(window),profile,
        colors->present?colors->channel:palette.highlight,&palette);
}
