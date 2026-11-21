#include "client_bar.h"

static void paint_button(HDC dc,RECT r,enum classic_button_role role,UINT flags,BOOL zoom,BOOL child,int u,
    const style_window_palette *p,const classic_caption_art *art,const style_caption_presentation *presentation)
{
    const COLORREF palette[]={p->edge,p->light,p->shadow,p->face};
    const style_caption_raster raster=style_caption_button_raster(art,role,zoom,child,flags);
    style_caption_button_paint(dc,r,role,zoom,flags,u,presentation,&raster,palette);
}

void style_menu_mdi_paint(HDC dc,RECT bounds,unsigned role,UINT flags,int unit,
    const style_window_palette *palette,const classic_caption_art *art,
    const style_caption_presentation *presentation)
{
    BOOL system=role==CLASSIC_MDI_SYSTEM;
    enum classic_button_role button=system?CLASSIC_BUTTON_SYSTEM:role==CLASSIC_MDI_CLOSE?CLASSIC_BUTTON_CLOSE:
        role==CLASSIC_MDI_MINIMIZE?CLASSIC_BUTTON_MINIMIZE:CLASSIC_BUTTON_MAXIMIZE;
    paint_button(dc,bounds,button,flags,role==CLASSIC_MDI_RESTORE,system,unit,palette,art,presentation);
}

void style_menu_client_bar_paint(const style_menu_bar_snapshot *bar,HDC dc,HFONT font,UINT dpi,
    const style_window_palette *palette,const classic_menu_colors *colors,const style_menu_metrics *metrics,
    const classic_caption_art *art,const style_caption_presentation *presentation)
{
    UINT i;
    for(i=0;i<bar->count;++i){
        UINT state=0;
        if(bar->items[i].state&MFS_HILITE)state|=ODS_SELECTED;
        if(bar->items[i].state&MFS_DISABLED)state|=ODS_DISABLED;
        if(bar->items[i].owner_draw){
            if(!style_menu_bar_draw_owned(bar,i,dc))return;
        }
        else if(bar->items[i].control){
            unsigned role=style_menu_mdi_role(bar->items[i].control);
            unsigned inactive=metrics && metrics->mdi_inactive?metrics->mdi_inactive:CLASSIC_MDI_ALL;
            UINT flags=(state&ODS_SELECTED)?DFCS_PUSHED:0;
            if((state&ODS_DISABLED) && (inactive&role))flags|=DFCS_INACTIVE;
            style_menu_mdi_paint(dc,bar->rects[i],role,flags,
                max(1,MulDiv(1,(int)dpi,96)),palette,art,presentation);
        }
        else {
            style_menu_columns columns={0};
            if(style_menu_columns_read(dc,font,dpi,bar->items[i].text,&columns))
                style_menu_paint_row(dc,&bar->rects[i],font,dpi,palette,bar->items[i].text,
                    state,STYLE_MENU_BAR,FALSE,&columns,colors,metrics,NULL,NULL);
        }
    }
}
