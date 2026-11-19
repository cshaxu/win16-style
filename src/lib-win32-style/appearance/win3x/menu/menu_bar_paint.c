#include "menu_bar.h"

void win16_style_bar_paint(const style_menu_bar_snapshot *bar,HDC dc,HFONT font,UINT dpi,
    const win16_style_palette *palette,const classic_menu_colors *colors,const win16_caption_art *art)
{
    UINT i;
    for(i=0;i<bar->count;++i){
        UINT state=0;
        if(bar->items[i].state&MFS_HILITE)state|=ODS_SELECTED;
        if(bar->items[i].state&MFS_DISABLED)state|=ODS_DISABLED;
        if(bar->items[i].owner_draw){
            if(!style_menu_bar_draw_owned(bar,i,dc))return;
        }
        else if(bar->items[i].control)win16_style_merged_control(dc,bar->rects[i],bar->items[i].control==HBMMENU_SYSTEM,
            (state&ODS_SELECTED)!=0,max(1,MulDiv(1,(int)dpi,96)),palette,art);
        else {
            win16_style_menu_columns columns={0};
            if(win16_style_menu_columns_read(dc,font,dpi,bar->items[i].text,&columns))
                win16_style_menu_text_columns(dc,&bar->rects[i],font,dpi,palette,bar->items[i].text,
                    state,WIN16_STYLE_MENU_BAR,FALSE,&columns,colors,NULL,NULL);
        }
    }
}
