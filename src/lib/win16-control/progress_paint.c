#include "progress_paint.h"
#include "control_protocol.h"
#include <commctrl.h>
#include <stdint.h>

BOOL win16_style_progress_supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return ntw16s_control_protocol_is(w,PROGRESS_CLASSW) && !(style&0xffffu&~(PBS_SMOOTH|PBS_VERTICAL|PBS_SMOOTHREVERSE)) && !(style&(PBS_MARQUEE|WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}
BOOL win16_style_progress_changes(UINT m)
{
    return m==PBM_SETRANGE || m==PBM_SETRANGE32 || m==PBM_SETPOS || m==PBM_DELTAPOS ||
        m==PBM_SETSTEP || m==PBM_STEPIT || m==PBM_SETBARCOLOR || m==PBM_SETBKCOLOR ||
        m==PBM_SETSTATE || m==PBM_SETMARQUEE || m==WM_STYLECHANGED || m==WM_SIZE || m==WM_ENABLE;
}
BOOL win16_style_progress_paint(HWND w,HDC dc,const win16_style_palette *palette,const BOOL *cancelled,const unsigned *epoch)
{
    if(!dc || !palette || !cancelled || !epoch || *cancelled || !win16_style_progress_supported(w))return FALSE;
    unsigned version=*epoch;RECT r;PBRANGE range={0};
    if(!GetClientRect(w,&r) || r.right<1 || r.bottom<1 || r.right>32767 || r.bottom>32767)return FALSE;
    SendMessageW(w,PBM_GETRANGE,0,(LPARAM)&range);
    if(*cancelled || version!=*epoch)return FALSE;
    int position=(int)(INT_PTR)SendMessageW(w,PBM_GETPOS,0,0);
    if(*cancelled || version!=*epoch)return FALSE;
    LRESULT state=SendMessageW(w,PBM_GETSTATE,0,0);
    if(*cancelled || version!=*epoch)return FALSE;
    COLORREF bar=(COLORREF)SendMessageW(w,PBM_GETBARCOLOR,0,0);
    if(*cancelled || version!=*epoch)return FALSE;
    COLORREF background=(COLORREF)SendMessageW(w,PBM_GETBKCOLOR,0,0);
    if(*cancelled || version!=*epoch || !win16_style_progress_supported(w) ||
        range.iLow>range.iHigh || position<range.iLow || position>range.iHigh ||
        state<PBST_NORMAL || state>PBST_PAUSED ||
        ((bar&0xff000000u) && bar!=CLR_DEFAULT) || ((background&0xff000000u) && background!=CLR_DEFAULT))return FALSE;
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    if(background==CLR_DEFAULT)background=palette->light;
    if(bar==CLR_DEFAULT)bar=palette->active;
    if(state==PBST_ERROR)bar=RGB(192,0,0);
    if(state==PBST_PAUSED)bar=RGB(192,192,0);
    if(!IsWindowEnabled(w))bar=palette->shadow;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);SetDCBrushColor(dc,background);FillRect(dc,&r,brush);
    SetDCBrushColor(dc,palette->edge);
    for(int i=0;i<u && r.right>r.left && r.bottom>r.top;++i){FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);}
    if(r.right>r.left && r.bottom>r.top){
        BOOL vertical=(style&PBS_VERTICAL)!=0;int length=vertical?r.bottom-r.top:r.right-r.left;
        int64_t span=(int64_t)range.iHigh-range.iLow;
        int filled=span?(int)(((int64_t)position-range.iLow)*length/span):0;
        SetDCBrushColor(dc,bar);
        if(style&PBS_SMOOTH){RECT fill=r;if(vertical)fill.top=fill.bottom-filled;else fill.right=fill.left+filled;FillRect(dc,&fill,brush);}
        else {int chunk=max(1,(vertical?r.right-r.left:r.bottom-r.top)*3/4),gap=2*u;
            for(int at=0;at<filled;at+=chunk+gap){RECT fill=r;int end=min(at+chunk,filled);
                if(vertical){fill.top=r.bottom-end;fill.bottom=r.bottom-at;}else{fill.left=r.left+at;fill.right=r.left+end;}
                FillRect(dc,&fill,brush);
            }
        }
    }
    RestoreDC(dc,saved);return TRUE;
}
