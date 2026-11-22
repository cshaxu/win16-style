#include "toolbar_text.h"
#include "control_protocol.h"
#include "../graphics/paint_frame.h"
#include <wchar.h>
BOOL style_toolbar_text_read(HWND w,int index,style_toolbar_text_item *item,BOOL (*accept)(HWND,const TBBUTTON *))
{
    TBBUTTONINFOW info={0};int capacity=ARRAYSIZE(item->text);
    ZeroMemory(&item->button,sizeof(item->button));
    if(!SendMessageW(w,TB_GETBUTTON,index,(LPARAM)&item->button) || !accept(w,&item->button))return FALSE;
    /* No unbounded TB_GETBUTTONTEXT preflight: text may change reentrantly. */
    info.cbSize=sizeof(info);info.dwMask=TBIF_TEXT;info.pszText=item->text;info.cchText=capacity;
    item->text[0]=0;item->text[capacity-1]=0;
    if(SendMessageW(w,TB_GETBUTTONINFOW,item->button.idCommand,(LPARAM)&info)<0 || !item->text[0] ||
        wcsnlen(item->text,(size_t)capacity)>=(size_t)(capacity-1) || !SendMessageW(w,TB_GETITEMRECT,index,(LPARAM)&item->rect) ||
        item->rect.right<=item->rect.left || item->rect.bottom<=item->rect.top)return FALSE;
    return TRUE;
}
BOOL style_toolbar_text_supported(HWND w,BOOL (*accept)(HWND,const TBBUTTON *))
{
    LRESULT count=SendMessageW(w,TB_BUTTONCOUNT,0,0);style_toolbar_text_item item;
    if(count<0 || count>1024 || (style_control_protocol_runtime_toolbar(w) && count>64))return FALSE;
    for(int i=0;i<count;++i)if(!style_toolbar_text_read(w,i,&item,accept) ||
        (style_control_protocol_runtime_toolbar(w) && SendMessageW(w,TB_COMMANDTOINDEX,item.button.idCommand,0)!=i))return FALSE;
    return TRUE;
}
BOOL style_toolbar_text_find(HWND w,DWORD_PTR command,style_toolbar_text_item *item,BOOL (*accept)(HWND,const TBBUTTON *))
{
    LRESULT count=SendMessageW(w,TB_BUTTONCOUNT,0,0);
    for(int i=0;i<count;++i){
        if(!style_toolbar_text_read(w,i,item,accept))return FALSE;
        if((DWORD_PTR)item->button.idCommand==command)return TRUE;
    }
    return FALSE;
}

/* A deliberately small Toolbar contract: text-only, regular/check buttons.
 * Images, split/drop-down buttons, separators and owner-draw variants retain
 * native rendering rather than losing an OCX-owned image list or command UI. */
static BOOL item_accepted(HWND w,const TBBUTTON *item)
{
    static const BYTE permitted=BTNS_AUTOSIZE|BTNS_CHECK|BTNS_GROUP|BTNS_CHECKGROUP|BTNS_NOPREFIX|BTNS_SHOWTEXT;
    return item->iBitmap==I_IMAGENONE && (item->idCommand || style_control_protocol_runtime_toolbar(w)) &&
        !(item->fsStyle&~permitted) && !(item->fsStyle&BTNS_SEP);
}
BOOL style_toolbar_outline_supported(HWND w)
{
    if(!style_control_protocol_toolbar(w) || (GetWindowLongPtrW(w,GWL_EXSTYLE)&
        (WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING)))return FALSE;
    return style_toolbar_text_supported(w,item_accepted);
}
BOOL style_toolbar_outline_part(HWND w,const NMCUSTOMDRAW *draw,const void *context)
{
    const style_toolbar_outline_view *view=context;
    style_toolbar_text_item item;RECT rect,client,inner;int saved,unit;HFONT source,font,old;
    if(!draw || !draw->hdc || !view || draw->dwDrawStage!=CDDS_ITEMPREPAINT || !style_toolbar_outline_supported(w) ||
        !GetClientRect(w,&client))return FALSE;
    const style_window_palette *palette=view->palette;
    const style_frame_colors *frame_colors=view->frame;
    const classic_bar_colors *colors=view->colors;
    COLORREF face=classic_bar_color(colors,CLASSIC_BAR_FACE,frame_colors->present?frame_colors->face:palette->face);
    COLORREF shadow=classic_bar_color(colors,CLASSIC_BAR_SHADOW,frame_colors->present?frame_colors->shadow:palette->shadow);
    COLORREF edge=classic_bar_color(colors,CLASSIC_BAR_EDGE,frame_colors->present?frame_colors->edge:palette->edge);
    COLORREF ink=classic_bar_color(colors,CLASSIC_BAR_TEXT,edge);
    COLORREF pressed=classic_bar_color(colors,CLASSIC_BAR_PRESSED,shadow);
    if(!style_toolbar_text_find(w,draw->dwItemSpec,&item,item_accepted))return FALSE;
    rect=item.rect;
    if(!IntersectRect(&inner,&client,&rect) || !EqualRect(&inner,&rect))return FALSE;
    saved=SaveDC(draw->hdc);if(!saved)return FALSE;
    IntersectClipRect(draw->hdc,rect.left,rect.top,rect.right,rect.bottom);
    BOOL pushed=(draw->uItemState&(CDIS_SELECTED|CDIS_CHECKED))!=0;
    unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    style_relief_palette frame={.dark=edge};
    inner=style_paint_outlined_face(draw->hdc,rect,unit,FALSE,FALSE,pushed?pressed:face,&frame);
    if(pushed)OffsetRect(&inner,unit,unit);
    InflateRect(&inner,-2*unit,-unit);
    source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    font=view->font_points?view->acquire_font(source,view->font_points):NULL;
    if(view->font_points)source=NULL;
    old=SelectObject(draw->hdc,font?font:source?source:GetStockObject(SYSTEM_FONT));
    SetBkMode(draw->hdc,TRANSPARENT);SetTextColor(draw->hdc,(draw->uItemState&CDIS_DISABLED)?shadow:ink);
    DrawTextW(draw->hdc,item.text,-1,&inner,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
    SelectObject(draw->hdc,old);view->release_font(font);RestoreDC(draw->hdc,saved);return TRUE;
}
