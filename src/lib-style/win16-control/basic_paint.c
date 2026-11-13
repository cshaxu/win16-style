#include "basic_paint.h"
#include "../common/paint_frame.h"
#include "combo_glyph.h"
#include "../common/pixel_grid.h"

void win16_control_combo_arrow(HDC dc,int left,int top,int dpi)
{
    const style_pixel_grid grid={&combo_arrow[0][0],7,9,8,1,NULL,{left,top},{dpi,dpi},{96,96}};
    style_paint_pixel_grid(dc,&grid);
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
    const COLORREF colors[4]={p->edge,p->light,p->shadow,p->face};
    const style_pixel_grid grid={&bits[0][0],17,17,18,-1,colors,{r.left,r.top},{r.right-r.left,r.bottom-r.top},{17,17}};
    style_paint_pixel_grid(dc,&grid);
}
