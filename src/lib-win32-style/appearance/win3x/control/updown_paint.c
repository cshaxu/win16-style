#include "updown_paint.h"

BOOL win16_control_updown_paint(HWND window,const win16_style_palette *palette,ntw16s_updown_glyph *const *glyphs,HDC dc,const RECT *bounds,const RECT *clip,unsigned direction,BOOL pressed,BOOL disabled)
{
    RECT client,r=*bounds;if(!GetClientRect(window,&client) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom ||
        r.left>=r.right || r.top>=r.bottom || r.right>32767 || r.bottom>32767)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
    IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
    win16_style_palette p=*palette;COLORREF colors[4]={p.edge,p.light,p.shadow,p.face};
    HBRUSH brush=GetStockObject(DC_BRUSH);int unit=max(1,MulDiv(1,(int)GetDpiForWindow(window),96));
    SetDCBrushColor(dc,p.face);FillRect(dc,&r,brush);SetDCBrushColor(dc,p.edge);
    for(int i=0;i<unit && r.left<r.right && r.top<r.bottom;++i){FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);}
    if(r.left<r.right && r.top<r.bottom){
        RECT strip=r;SetDCBrushColor(dc,pressed?p.shadow:p.light);strip.bottom=min(r.top+unit,r.bottom);FillRect(dc,&strip,brush);
        strip=r;strip.right=min(r.left+unit,r.right);FillRect(dc,&strip,brush);
        SetDCBrushColor(dc,pressed?p.light:p.shadow);strip=r;strip.top=max(r.bottom-unit,r.top);FillRect(dc,&strip,brush);
        strip=r;strip.left=max(r.right-unit,r.left);FillRect(dc,&strip,brush);
        InflateRect(&r,-unit,-unit);
        int side=min(MulDiv(11,(int)GetDpiForWindow(window),96),min(r.right-r.left,r.bottom-r.top));
        if(side>0){
            /* The original arrow interior scales equally in both axes. The
             * native hit rectangle may be wide/short; never stretch the art. */
            int left=r.left+(r.right-r.left-side)/2,top=r.top+(r.bottom-r.top-side)/2;
            const char (*bits)[18]=*glyphs[direction+(disabled?8:pressed?4:0)];
            for(int y=0;y<11;++y)for(int x=0;x<11;++x){
                RECT pixel={left+MulDiv(x,side,11),top+MulDiv(y,side,11),left+MulDiv(x+1,side,11),top+MulDiv(y+1,side,11)};
                SetDCBrushColor(dc,colors[bits[y+3][x+3]-'0']);FillRect(dc,&pixel,brush);
            }
        }
    }
    RestoreDC(dc,saved);return TRUE;
}
