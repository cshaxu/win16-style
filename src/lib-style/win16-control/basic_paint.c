#include "basic_paint.h"
#include "combo_glyph.h"

void win16_control_combo_arrow(HDC dc,int left,int top,int dpi)
{
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    for(int y=0;y<9;++y)for(int x=0;x<7;++x)if(combo_arrow[y][x]=='0'){
        RECT pixel={left+MulDiv(x,dpi,96),top+MulDiv(y,dpi,96),
            left+MulDiv(x+1,dpi,96),top+MulDiv(y+1,dpi,96)};
        FillRect(dc,&pixel,brush);
    }
}

RECT win16_control_button_border(HDC dc,RECT r,const win16_style_palette *palette,int u,int border,BOOL pushed)
{
    RECT bounds=r,corner;
    style_paint_solid_frame(dc,bounds,palette->edge,border);
    SetDCBrushColor(dc,palette->light);
    SetRect(&corner,r.left,r.top,r.left+u,r.top+u);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,r.right-r.left-u,0);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,0,r.bottom-r.top-u);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,-(r.right-r.left-u),0);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    InflateRect(&bounds,-border,-border);
    RECT strip=bounds;int band=pushed?u:2*u;
    SetDCBrushColor(dc,pushed?palette->shadow:palette->light);
    strip.bottom=strip.top+band;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
    strip=bounds;strip.right=strip.left+band;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
    if(!pushed){SetDCBrushColor(dc,palette->shadow);
        for(int i=0;i<2;++i){strip=bounds;InflateRect(&strip,-i*u,-i*u);
            strip.top=strip.bottom-u;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
            strip=bounds;InflateRect(&strip,-i*u,-i*u);
            strip.left=strip.right-u;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));}}
    return bounds;
}
void win16_control_scroll_arrow(HDC dc,RECT r,const char bits[17][18],const win16_style_palette *p)
{
    int x,y;COLORREF colors[4]={p->edge,p->light,p->shadow,p->face};RECT pixel;
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    for(y=0;y<17;++y)for(x=0;x<17;++x){
        SetDCBrushColor(dc,colors[bits[y][x]-'0']);
        SetRect(&pixel,r.left+MulDiv(x,r.right-r.left,17),r.top+MulDiv(y,r.bottom-r.top,17),
            r.left+MulDiv(x+1,r.right-r.left,17),r.top+MulDiv(y+1,r.bottom-r.top,17));
        FillRect(dc,&pixel,brush);
    }
}
