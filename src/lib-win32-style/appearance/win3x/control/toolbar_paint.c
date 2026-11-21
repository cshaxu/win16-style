#include "toolbar_paint.h"
#include "control_state.h"
#include "../../../application/control/control_protocol.h"
#include "../common/font.h"
#include "lib-win32-style/application/control/toolbar_text.h"

/* A deliberately small Toolbar contract: text-only, regular/check buttons.
 * Images, split/drop-down buttons, separators and owner-draw variants retain
 * native rendering rather than losing an OCX-owned image list or command UI. */
static BOOL item_accepted(HWND w,const TBBUTTON *item)
{
    static const BYTE permitted=BTNS_AUTOSIZE|BTNS_CHECK|BTNS_GROUP|BTNS_CHECKGROUP|BTNS_NOPREFIX|BTNS_SHOWTEXT;
    return item->iBitmap==I_IMAGENONE && (item->idCommand || style_control_protocol_runtime_toolbar(w)) &&
        !(item->fsStyle&~permitted) && !(item->fsStyle&BTNS_SEP);
}
BOOL win16_style_toolbar_supported(HWND w)
{
    if(!style_control_protocol_toolbar(w) || (GetWindowLongPtrW(w,GWL_EXSTYLE)&
        (WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING)))return FALSE;
    return style_toolbar_text_supported(w,item_accepted);
}
BOOL win16_style_toolbar_part(HWND w,const NMCUSTOMDRAW *draw,const void *context)
{
    const control_style *control=context;
    style_toolbar_text_item item;RECT rect,client,inner;int saved,unit;HFONT source,font,old;
    if(!draw || !draw->hdc || !control || draw->dwDrawStage!=CDDS_ITEMPREPAINT || !win16_style_toolbar_supported(w) ||
        !GetClientRect(w,&client))return FALSE;
    win16_style_palette palette=control->palette;
    if(control->frame_colors.present){
        palette.face=control->frame_colors.face;
        palette.shadow=control->frame_colors.shadow;
        palette.edge=control->frame_colors.edge;
    }
    const classic_bar_colors *colors=&control->toolbar_colors;
    palette.face=classic_bar_color(colors,CLASSIC_BAR_FACE,palette.face);
    palette.shadow=classic_bar_color(colors,CLASSIC_BAR_SHADOW,palette.shadow);
    palette.edge=classic_bar_color(colors,CLASSIC_BAR_EDGE,palette.edge);
    COLORREF ink=classic_bar_color(colors,CLASSIC_BAR_TEXT,palette.edge);
    COLORREF pressed=classic_bar_color(colors,CLASSIC_BAR_PRESSED,palette.shadow);
    if(!style_toolbar_text_find(w,draw->dwItemSpec,&item,item_accepted))return FALSE;
    rect=item.rect;
    if(!IntersectRect(&inner,&client,&rect) || !EqualRect(&inner,&rect))return FALSE;
    saved=SaveDC(draw->hdc);if(!saved)return FALSE;
    IntersectClipRect(draw->hdc,rect.left,rect.top,rect.right,rect.bottom);
    BOOL pushed=(draw->uItemState&(CDIS_SELECTED|CDIS_CHECKED))!=0;
    unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    style_relief_palette frame={.dark=palette.edge};
    inner=style_paint_outlined_face(draw->hdc,rect,unit,FALSE,FALSE,pushed?pressed:palette.face,&frame);
    if(draw->uItemState&(CDIS_SELECTED|CDIS_CHECKED))OffsetRect(&inner,unit,unit);
    InflateRect(&inner,-2*unit,-unit);
    source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    font=control->controls.control_font_points?win16_style_system_acquire(source,control->controls.control_font_points):NULL;
    if(control->controls.control_font_points)source=NULL;
    old=SelectObject(draw->hdc,font?font:source?source:GetStockObject(SYSTEM_FONT));
    SetBkMode(draw->hdc,TRANSPARENT);SetTextColor(draw->hdc,(draw->uItemState&CDIS_DISABLED)?palette.shadow:ink);
    DrawTextW(draw->hdc,item.text,-1,&inner,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
    SelectObject(draw->hdc,old);win16_style_system_release(font);RestoreDC(draw->hdc,saved);return TRUE;
}
