#include "basic_paint.h"
#include "../../../application/graphics/paint_frame.h"
#include "../resources/combo_glyph.h"
#include "../resources/scroll_assets.h"
#include "../../../application/graphics/pixel_grid.h"

void win16_control_combo_arrow(HDC dc,int left,int top,int dpi)
{
    const style_pixel_grid grid={&combo_arrow[0][0],7,9,8,1,NULL,{left,top},{dpi,dpi},{96,96}};
    style_paint_pixel_grid(dc,&grid);
}

style_relief_palette win16_control_push_palette(const win16_style_palette *palette)
{
    return (style_relief_palette){.face=palette->face,.light=palette->face,.highlight=palette->light,
        .shadow=palette->shadow,.dark=palette->edge,.text=palette->edge,.window=palette->light};
}

void win16_control_scroll_arrow(HDC dc,RECT r,const char bits[17][18],const win16_style_palette *p)
{
    const COLORREF colors[4]={p->edge,p->light,p->shadow,p->face};
    const style_pixel_grid grid={&bits[0][0],17,17,18,-1,colors,{r.left,r.top},{r.right-r.left,r.bottom-r.top},{17,17}};
    style_paint_pixel_grid(dc,&grid);
}

void win16_control_scroll_arrow_art(HDC dc,RECT bounds,UINT state,const win16_style_palette *palette,
    const style_scroll_arrow_art *art)
{
    if(art && art->source){
        style_relief_palette colors=win16_control_push_palette(palette);
        if(style_paint_scroll_arrow(dc,&bounds,state,&colors,art))return;
    }
    unsigned direction=state&0xffu;
    unsigned index=(state&DFCS_INACTIVE)?2u:(state&DFCS_PUSHED)?1u:0u;
    const char (*glyph)[18]=win16_control_scroll_glyph((enum win16_control_scroll_asset)
        (index*STYLE_SCROLL_ARROW_DIRECTIONS+direction));
    if(glyph)win16_control_scroll_arrow(dc,bounds,glyph,palette);
}
