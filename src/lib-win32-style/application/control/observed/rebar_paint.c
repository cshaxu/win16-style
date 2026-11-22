/* The horizontal grabber in rebar.c is a one-layer raised edge.
 * Confirm the native band exposes a grabber before drawing into its header. */
#include "bars.h"
#include "lib-win32-style/application/graphics/paint_frame.h"

BOOL style_observed_rebar_supported(HWND window)
{
    return !(GetWindowLongPtrW(window,GWL_STYLE)&CCS_VERT);
}

void style_observed_rebar_grip(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    const style_observed_bar_draw *parameters=context;
    const style_instance *instance=parameters->instance;
    UINT dpi=GetDpiForWindow(window);
    if(!draw->hdc || !dpi)return;
    int border=GetSystemMetricsForDpi(SM_CXBORDER,dpi);
    int edge=GetSystemMetricsForDpi(SM_CXEDGE,dpi);
    RECT grip={draw->rc.left+2*border,draw->rc.top+edge,draw->rc.left+5*border,draw->rc.bottom-edge};
    if(grip.right>draw->rc.right || IsRectEmpty(&grip))return;
    RBHITTESTINFO hit={.pt={grip.left,(grip.top+grip.bottom)/2}};
    LRESULT index=SendMessageW(window,RB_HITTEST,0,(LPARAM)&hit);
    if(index<0 || hit.flags!=RBHT_GRABBER || instance->detached)return;
    REBARBANDINFOW band={.cbSize=REBARBANDINFOW_V3_SIZE,.fMask=RBBIM_ID|RBBIM_BACKGROUND|RBBIM_COLORS};
    if(!SendMessageW(window,RB_GETBANDINFOW,(WPARAM)index,(LPARAM)&band) ||
        instance->detached || band.wID!=draw->dwItemSpec || band.hbmBack)return;
    COLORREF background=band.clrBack;
    if(background==CLR_DEFAULT)background=(COLORREF)SendMessageW(window,RB_GETBKCOLOR,0,0);
    if(instance->detached || (background!=CLR_DEFAULT && background!=GetSysColor(COLOR_BTNFACE)))return;
    style_paint_relief(draw->hdc,&grip,BDR_RAISEDINNER,BF_RECT|BF_MIDDLE,&parameters->palette);
}
