#include "trackbar_paint.h"
#include "control_protocol.h"
#include "../graphics/dc_geometry.h"

static BOOL trackbar_supported(HWND window,DWORD extra)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    DWORD ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    const DWORD base=TBS_AUTOTICKS|TBS_VERT|TBS_TOP|TBS_BOTH|TBS_NOTICKS|TBS_FIXEDLENGTH;
    return !(style&(0xffffu&~(base|extra))) && !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

BOOL style_trackbar_bound_supported(HWND window)
{
    DWORD extra=style_control_protocol_runtime_trackbar(window)?TBS_ENABLESELRANGE:0;
    return style_control_protocol_is(window,TRACKBAR_CLASSW) && trackbar_supported(window,extra);
}

BOOL style_trackbar_native_supported(HWND window)
{return trackbar_supported(window,TBS_NOTHUMB);}

static BOOL drawable_part(HWND window,const NMCUSTOMDRAW *draw,DWORD style)
{
    RECT r=draw->rc,client;
    BOOL part=draw->dwItemSpec==TBCD_CHANNEL ||
        (draw->dwItemSpec==TBCD_THUMB && !(style&TBS_NOTHUMB));
    return draw->hdc && part && GetClientRect(window,&client) && !IsRectEmpty(&r) &&
        r.left>=client.left && r.top>=client.top && r.right<=client.right && r.bottom<=client.bottom;
}

BOOL style_trackbar_notify_part(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    const style_trackbar_parts *parts=context;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    if(!drawable_part(window,draw,style) || (parts->supported && !parts->supported(window)))return FALSE;
    const classic_trackbar_colors *colors=&parts->axes[(style&TBS_VERT)!=0];
    style_relief_palette paint=parts->palette;
    if(colors->present){
        paint.face=colors->thumb;
        paint.dark=colors->edge;
    }
    return style_trackbar_paint(draw->hdc,draw->rc,style,GetDpiForWindow(window),
        draw->dwItemSpec==TBCD_CHANNEL,(draw->uItemState&CDIS_SELECTED)!=0,
        !IsWindowEnabled(window),parts->profile,colors->present?colors->channel:paint.highlight,&paint);
}

BOOL style_trackbar_notify_runtime_border(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    const style_trackbar_parts *parts=context;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    if(!style_control_protocol_runtime_trackbar(window) || !style_trackbar_bound_supported(window) ||
        !drawable_part(window,draw,style) || !style_dc_unmirrored_origin_pixels(draw->hdc))return FALSE;
    int saved=SaveDC(draw->hdc);
    if(!saved)return FALSE;
    const classic_trackbar_colors *colors=&parts->axes[(style&TBS_VERT)!=0];
    SetDCBrushColor(draw->hdc,colors->present?colors->edge:parts->palette.dark);
    FrameRect(draw->hdc,&draw->rc,(HBRUSH)GetStockObject(DC_BRUSH));
    RestoreDC(draw->hdc,saved);
    return TRUE;
}
