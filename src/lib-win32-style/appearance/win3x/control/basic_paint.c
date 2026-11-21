#include "basic_paint.h"
#include "../../../application/graphics/paint_frame.h"
#include "../../../application/graphics/pixel_grid.h"

void win16_control_combo_arrow(HDC dc,int left,int top,int dpi,const classic_combo_arrow_art *art)
{
    if(art && art->present){
        char pixels[CLASSIC_COMBO_ARROW_HEIGHT*CLASSIC_COMBO_ARROW_WIDTH];
        for(unsigned y=0;y<CLASSIC_COMBO_ARROW_HEIGHT;++y)
            for(unsigned x=0;x<CLASSIC_COMBO_ARROW_WIDTH;++x)
                pixels[y*CLASSIC_COMBO_ARROW_WIDTH+x]=art->rows[y]&(1u<<x)?'0':'1';
        const style_pixel_grid grid={pixels,CLASSIC_COMBO_ARROW_WIDTH,CLASSIC_COMBO_ARROW_HEIGHT,
            CLASSIC_COMBO_ARROW_WIDTH,1,NULL,{left,top},{dpi,dpi},{96,96}};
        style_paint_pixel_grid(dc,&grid);
        return;
    }
    int width=MulDiv(CLASSIC_COMBO_ARROW_WIDTH,dpi,96);
    int height=MulDiv(CLASSIC_COMBO_ARROW_HEIGHT,dpi,96);
    POINT triangle[]={{left,top+height/3},{left+width,top+height/3},{left+width/2,top+2*height/3}};
    HPEN pen=SelectObject(dc,GetStockObject(NULL_PEN));
    HBRUSH brush=SelectObject(dc,GetStockObject(DC_BRUSH));
    Polygon(dc,triangle,ARRAYSIZE(triangle));
    if(brush)SelectObject(dc,brush);
    if(pen)SelectObject(dc,pen);
}

style_relief_palette win16_control_push_palette(const win16_style_palette *palette)
{
    return (style_relief_palette){.face=palette->face,.light=palette->face,.highlight=palette->light,
        .shadow=palette->shadow,.dark=palette->edge,.text=palette->edge,.window=palette->light};
}

void win16_control_scroll_arrow_art(HDC dc,RECT bounds,UINT state,const win16_style_palette *palette,
    const style_scroll_arrow_art *art)
{
    const style_relief_palette colors=win16_control_push_palette(palette);
    (void)style_paint_scroll_arrow(dc,&bounds,state,&colors,art);
}
