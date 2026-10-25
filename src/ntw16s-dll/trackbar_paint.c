#include "trackbar_paint.h"
#include "control_protocol.h"

BOOL win16_style_trackbar_supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return ntw16s_control_protocol_is(w,TRACKBAR_CLASSW) &&
        !(style&0xffffu&~(TBS_AUTOTICKS|TBS_VERT|TBS_TOP|TBS_BOTH|TBS_NOTICKS|TBS_FIXEDLENGTH)) &&
        !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}
BOOL win16_style_trackbar_part(HWND w,const NMCUSTOMDRAW *draw,const win16_style_palette *palette)
{
    if(!draw->hdc || (draw->dwItemSpec!=TBCD_CHANNEL && draw->dwItemSpec!=TBCD_THUMB) ||
        !win16_style_trackbar_supported(w))return FALSE;
    RECT r=draw->rc,client;
    if(!GetClientRect(w,&client) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom ||
        r.right<=r.left || r.bottom<=r.top)return FALSE;
    int saved=SaveDC(draw->hdc);if(!saved)return FALSE;
    HBRUSH brush=GetStockObject(DC_BRUSH);int unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    SetDCBrushColor(draw->hdc,draw->dwItemSpec==TBCD_CHANNEL?palette->light:palette->face);FillRect(draw->hdc,&r,brush);
    SetDCBrushColor(draw->hdc,palette->edge);
    for(int i=0;i<unit && r.left<r.right && r.top<r.bottom;++i){FrameRect(draw->hdc,&r,brush);InflateRect(&r,-1,-1);}
    if(draw->dwItemSpec==TBCD_THUMB && r.right>r.left && r.bottom>r.top){
        BOOL pressed=(draw->uItemState&CDIS_SELECTED)!=0;RECT strip=r;
        SetDCBrushColor(draw->hdc,pressed?palette->shadow:palette->light);
        strip.bottom=min(r.top+unit,r.bottom);FillRect(draw->hdc,&strip,brush);
        strip=r;strip.right=min(r.left+unit,r.right);FillRect(draw->hdc,&strip,brush);
        SetDCBrushColor(draw->hdc,pressed?palette->light:palette->shadow);
        strip=r;strip.top=max(r.bottom-unit,r.top);FillRect(draw->hdc,&strip,brush);
        strip=r;strip.left=max(r.right-unit,r.left);FillRect(draw->hdc,&strip,brush);
    }
    RestoreDC(draw->hdc,saved);return TRUE;
}
