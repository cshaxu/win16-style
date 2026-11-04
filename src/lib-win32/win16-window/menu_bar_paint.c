#include "menu_bar.h"

void win16_style_bar_paint(const win16_style_bar *bar,HDC dc,HFONT font,UINT dpi,const win16_style_palette *palette)
{
    UINT i;
    for(i=0;i<bar->count;++i){
        UINT state=0;
        if(bar->items[i].state&MFS_HILITE)state|=ODS_SELECTED;
        if(bar->items[i].state&MFS_DISABLED)state|=ODS_DISABLED;
        if(bar->items[i].owner_draw){
            if(bar->items[i].state&MFS_DISABLED)state|=ODS_GRAYED;
            if(bar->items[i].state&MFS_CHECKED)state|=ODS_CHECKED;
            if(bar->items[i].state&MFS_DEFAULT)state|=ODS_DEFAULT;
            if(SendMessageW(bar->owner,WM_QUERYUISTATE,0,0)&UISF_HIDEACCEL)state|=ODS_NOACCEL;
            if(GetAncestor(GetForegroundWindow(),GA_ROOT)!=GetAncestor(bar->owner,GA_ROOT))state|=ODS_INACTIVE;
            if(!win16_style_bar_item_current(bar,i))return;
            DRAWITEMSTRUCT draw={.CtlType=ODT_MENU,.itemID=bar->items[i].id,
                .itemAction=ODA_DRAWENTIRE,.itemState=state,.hwndItem=(HWND)bar->menu,
                .hDC=dc,.rcItem=bar->rects[i],.itemData=bar->items[i].data};
            int saved=SaveDC(dc);if(!saved)continue;
            IntersectClipRect(dc,draw.rcItem.left,draw.rcItem.top,draw.rcItem.right,draw.rcItem.bottom);
            SendMessageW(bar->owner,WM_DRAWITEM,0,(LPARAM)&draw);
            RestoreDC(dc,saved);
            if(!IsWindow(bar->owner) || !IsMenu(bar->menu))return;
        }
        else if(bar->items[i].control)win16_style_merged_control(dc,bar->rects[i],bar->items[i].control==HBMMENU_SYSTEM,
            (state&ODS_SELECTED)!=0,max(1,MulDiv(1,(int)dpi,96)),palette);
        else win16_style_menu_text(dc,&bar->rects[i],font,dpi,palette,bar->items[i].text,state,WIN16_STYLE_MENU_BAR,FALSE);
    }
}
