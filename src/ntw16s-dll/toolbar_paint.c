#include "toolbar_paint.h"
#include "control_protocol.h"
#include "font.h"
#include <stdlib.h>

/* A deliberately small Toolbar contract: text-only, regular/check buttons.
 * Images, split/drop-down buttons, separators and owner-draw variants retain
 * native rendering rather than losing an OCX-owned image list or command UI. */
static BOOL button(HWND w,int index,TBBUTTON *out,WCHAR *text,int capacity,RECT *rect)
{
    static const BYTE permitted=BTNS_AUTOSIZE|BTNS_CHECK|BTNS_GROUP|BTNS_CHECKGROUP|BTNS_NOPREFIX|BTNS_SHOWTEXT;
    TBBUTTONINFOW info={0};
    ZeroMemory(out,sizeof(*out));
    if(!SendMessageW(w,TB_GETBUTTON,index,(LPARAM)out) || out->iBitmap!=I_IMAGENONE || !out->idCommand ||
        (out->fsStyle&~permitted) || (out->fsStyle&BTNS_SEP) || !text || capacity<2)return FALSE;
    /* TB_GETBUTTONTEXT has no capacity argument: even a length preflight
     * cannot prevent a reentrant text change from overflowing this buffer.
     * A full buffer is conservatively treated as truncated and stays native. */
    info.cbSize=sizeof(info);info.dwMask=TBIF_TEXT;info.pszText=text;info.cchText=capacity;
    text[0]=0;text[capacity-1]=0;
    if(SendMessageW(w,TB_GETBUTTONINFOW,out->idCommand,(LPARAM)&info)<0 || !text[0] ||
        wcsnlen(text,(size_t)capacity)>=(size_t)(capacity-1) || !SendMessageW(w,TB_GETITEMRECT,index,(LPARAM)rect) ||
        rect->right<=rect->left || rect->bottom<=rect->top)return FALSE;
    return TRUE;
}
BOOL win16_style_toolbar_supported(HWND w)
{
    LRESULT count;TBBUTTON item;WCHAR text[512];RECT rect;
    if(!ntw16s_control_protocol_toolbar(w) || (GetWindowLongPtrW(w,GWL_EXSTYLE)&
        (WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING)))return FALSE;
    count=SendMessageW(w,TB_BUTTONCOUNT,0,0);
    if(count<0 || count>1024)return FALSE;
    for(int i=0;i<count;++i)if(!button(w,i,&item,text,ARRAYSIZE(text),&rect))return FALSE;
    return TRUE;
}
static void fill(HDC dc,const RECT *r,COLORREF color)
{SetDCBrushColor(dc,color);FillRect(dc,r,GetStockObject(DC_BRUSH));}
BOOL win16_style_toolbar_part(HWND w,const NMCUSTOMDRAW *draw,const win16_style_palette *palette)
{
    LRESULT count;TBBUTTON item;WCHAR text[512];RECT rect,client,inner;int index=-1,saved,unit;HFONT source,font,old;
    if(!draw || !draw->hdc || !palette || draw->dwDrawStage!=CDDS_ITEMPREPAINT || !win16_style_toolbar_supported(w) ||
        !GetClientRect(w,&client))return FALSE;
    count=SendMessageW(w,TB_BUTTONCOUNT,0,0);
    for(int i=0;i<count;++i){
        if(!button(w,i,&item,text,ARRAYSIZE(text),&rect))return FALSE;
        if((DWORD_PTR)item.idCommand==draw->dwItemSpec){index=i;break;}
    }
    if(index<0 || !IntersectRect(&inner,&client,&rect) || !EqualRect(&inner,&rect))return FALSE;
    saved=SaveDC(draw->hdc);if(!saved)return FALSE;
    IntersectClipRect(draw->hdc,rect.left,rect.top,rect.right,rect.bottom);
    fill(draw->hdc,&rect,(draw->uItemState&(CDIS_SELECTED|CDIS_CHECKED))?palette->shadow:palette->face);
    unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));inner=rect;
    for(int i=0;i<unit && inner.right>inner.left && inner.bottom>inner.top;++i){
        SetDCBrushColor(draw->hdc,palette->edge);FrameRect(draw->hdc,&inner,GetStockObject(DC_BRUSH));InflateRect(&inner,-1,-1);
    }
    if(draw->uItemState&(CDIS_SELECTED|CDIS_CHECKED))OffsetRect(&inner,unit,unit);
    InflateRect(&inner,-2*unit,-unit);
    source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);font=win16_style_system_acquire(source);old=SelectObject(draw->hdc,font?font:GetStockObject(SYSTEM_FONT));
    SetBkMode(draw->hdc,TRANSPARENT);SetTextColor(draw->hdc,(draw->uItemState&CDIS_DISABLED)?palette->shadow:palette->edge);
    DrawTextW(draw->hdc,text,-1,&inner,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
    SelectObject(draw->hdc,old);win16_style_system_release(font);RestoreDC(draw->hdc,saved);return TRUE;
}
