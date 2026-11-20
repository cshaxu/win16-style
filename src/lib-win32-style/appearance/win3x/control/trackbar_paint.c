#include "trackbar_paint.h"
#include "control_state.h"
#include "../../../application/control/control_protocol.h"
#include "../../../application/graphics/dc_geometry.h"

BOOL win16_style_trackbar_supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    DWORD allowed=TBS_AUTOTICKS|TBS_VERT|TBS_TOP|TBS_BOTH|TBS_NOTICKS|TBS_FIXEDLENGTH;
    if(style_control_protocol_runtime_trackbar(w))allowed|=TBS_ENABLESELRANGE;
    return style_control_protocol_is(w,TRACKBAR_CLASSW) &&
        !(style&0xffffu&~allowed) &&
        !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}
BOOL win16_style_trackbar_runtime_border(HWND w,const NMCUSTOMDRAW *draw,const void *context)
{
    RECT client,r=draw->rc;
    if(!style_control_protocol_runtime_trackbar(w) || !win16_style_trackbar_supported(w) || !draw->hdc ||
       !style_dc_unmirrored_origin_pixels(draw->hdc) ||
       (draw->dwItemSpec!=TBCD_CHANNEL && draw->dwItemSpec!=TBCD_THUMB) || !GetClientRect(w,&client) ||
       r.left<client.left || r.top<client.top || r.right>client.right || r.bottom>client.bottom ||
       r.right<=r.left || r.bottom<=r.top)return FALSE;
    int saved=SaveDC(draw->hdc);if(!saved)return FALSE;
    const control_style *control=context;
    const classic_trackbar_colors *colors=&control->trackbar_colors[(GetWindowLongPtrW(w,GWL_STYLE)&TBS_VERT)!=0];
    SetDCBrushColor(draw->hdc,colors->present?colors->edge:control->palette.edge);
    FrameRect(draw->hdc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    RestoreDC(draw->hdc,saved);return TRUE;
}
BOOL win16_style_trackbar_part(HWND w,const NMCUSTOMDRAW *draw,const void *context)
{
    const control_style *control=context;
    const win16_style_palette *palette=&control->palette;
    if(!draw->hdc || (draw->dwItemSpec!=TBCD_CHANNEL && draw->dwItemSpec!=TBCD_THUMB) ||
        !win16_style_trackbar_supported(w))return FALSE;
    RECT r=draw->rc,client;
    if(!GetClientRect(w,&client) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom ||
        r.right<=r.left || r.bottom<=r.top)return FALSE;
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    const classic_trackbar_colors *colors=&control->trackbar_colors[(style&TBS_VERT)!=0];
    const style_relief_palette paint={
        .face=colors->present?colors->thumb:palette->face,.light=palette->light,
        .highlight=palette->light,.shadow=palette->shadow,.dark=colors->present?colors->edge:palette->edge
    };
    style_trackbar_profile profile=control->controls.trackbar?control->controls.trackbar:STYLE_TRACKBAR_OUTLINED;
    return style_trackbar_paint(draw->hdc,r,style,GetDpiForWindow(w),draw->dwItemSpec==TBCD_CHANNEL,
        (draw->uItemState&CDIS_SELECTED)!=0,!IsWindowEnabled(w),profile,
        colors->present?colors->channel:palette->light,&paint);
}
