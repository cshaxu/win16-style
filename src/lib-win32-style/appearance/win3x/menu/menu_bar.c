#include "menu_bar.h"
#include "lib-win32-style/application/menu/menu_input.h"
#include <windowsx.h>

int win16_control_menu_bar_item_screen(HWND w,const win16_style_layout *l,HFONT font,UINT dpi,LPARAM lp,BOOL *close)
{
    style_menu_bar_snapshot bar;POINT origin={l->menu.left,l->menu.top},point;RECT r;int item=-1;
    if(close)*close=FALSE;
    GetWindowRect(w,&r);point.x=GET_X_LPARAM(lp)-r.left;point.y=GET_Y_LPARAM(lp)-r.top;
    if(win16_style_bar_read_for_window(w,GetMenu(w),font,dpi,l,origin,l->menu.right-l->menu.left,&bar)){
        item=style_classic_menu_hit(bar.rects,bar.count,point);
        if(close && item>=0 && bar.items[item].control==HBMMENU_SYSTEM && bar.items[item].popup){
            UINT state=GetMenuState(bar.items[item].popup,SC_CLOSE,MF_BYCOMMAND);
            *close=state!=(UINT)-1 && !(state&(MF_DISABLED|MF_GRAYED));
        }
        style_menu_bar_free(&bar);
    }
    return item;
}
int win16_control_menu_bar_hit_screen(HWND w,const win16_style_layout *layout,
    HFONT font,UINT dpi,POINT point)
{
    style_menu_bar_snapshot bar;RECT outer;POINT origin;int hit;
    if(!layout || !GetWindowRect(w,&outer))return -3;
    point.x-=outer.left;point.y-=outer.top;
    if(PtInRect(&layout->buttons[CLASSIC_BUTTON_SYSTEM],point))return -2;
    if(!PtInRect(&layout->menu,point))return -3;
    origin.x=layout->menu.left;origin.y=layout->menu.top;
    if(!win16_style_bar_read_for_window(w,GetMenu(w),font,dpi,layout,origin,layout->menu.right-layout->menu.left,&bar))return -3;
    hit=style_classic_menu_hit(bar.rects,bar.count,point);style_menu_bar_free(&bar);
    return hit<0?-3:hit;
}
typedef struct bar_measure {HFONT font;UINT dpi;const win16_style_layout *layout;} bar_measure;

unsigned win16_menu_mdi_role(HBITMAP bitmap)
{
    if(bitmap==HBMMENU_SYSTEM)return CLASSIC_MDI_SYSTEM;
    if(bitmap==HBMMENU_MBAR_RESTORE)return CLASSIC_MDI_RESTORE;
    if(bitmap==HBMMENU_MBAR_MINIMIZE || bitmap==HBMMENU_MBAR_MINIMIZE_D)return CLASSIC_MDI_MINIMIZE;
    if(bitmap==HBMMENU_MBAR_CLOSE || bitmap==HBMMENU_MBAR_CLOSE_D)return CLASSIC_MDI_CLOSE;
    return 0;
}

static style_menu_bar_inclusion include_item(void *context,const MENUITEMINFOW *item,UINT *flags)
{
    if(item->fType&MFT_SEPARATOR)return STYLE_MENU_BAR_UNSUPPORTED;
    if(!item->hbmpItem)return STYLE_MENU_BAR_INCLUDE;
    unsigned role=win16_menu_mdi_role(item->hbmpItem);
    if(!role)return STYLE_MENU_BAR_UNSUPPORTED;
    const bar_measure *measure=context;
    unsigned visible=measure->layout->menu_metrics.mdi_buttons;
    if(visible && !(visible&role))return STYLE_MENU_BAR_SKIP;
    if(role!=CLASSIC_MDI_SYSTEM)*flags|=MFT_RIGHTJUSTIFY;
    return STYLE_MENU_BAR_INCLUDE;
}
static BOOL measure_item(void *context,HDC dc,const MENUITEMINFOW *item,SIZE *size)
{
    const bar_measure *measure=context;
    if(item->hbmpItem){*size=measure->layout->button_size;return TRUE;}
    return win16_style_menu_bar_measure(dc,measure->font,measure->dpi,item->dwTypeData,&measure->layout->menu_metrics,size);
}
BOOL win16_style_bar_read_for_window(HWND owner,HMENU menu,HFONT font,UINT dpi,
    const win16_style_layout *layout,POINT origin,int width,style_menu_bar_snapshot *bar)
{
    const style_menu_bar_profile profile={include_item,measure_item,FALSE};
    bar_measure measure={font,dpi,layout};
    if(!style_menu_bar_read(owner,menu,origin,width,&profile,&measure,bar))return FALSE;
    for(UINT i=0;i<bar->count;++i){
        style_menu_bar_item *item=&bar->items[i];
        if(item->control==HBMMENU_MBAR_MINIMIZE_D || item->control==HBMMENU_MBAR_CLOSE_D)
            item->state|=MFS_DISABLED;
    }
    return TRUE;
}
