#include "menu_bar.h"
#include "lib-win32-style/application/menu/menu_input.h"
#include <windowsx.h>

int win16_control_menu_bar_item_screen(HWND w,const win16_style_layout *l,HFONT font,UINT dpi,LPARAM lp,BOOL *close)
{
    style_menu_bar_snapshot bar;POINT origin={l->menu.left,l->menu.top},point;RECT r;int item=-1;
    if(close)*close=FALSE;
    GetWindowRect(w,&r);point.x=GET_X_LPARAM(lp)-r.left;point.y=GET_Y_LPARAM(lp)-r.top;
    if(win16_style_bar_read_for_window(w,GetMenu(w),font,dpi,origin,l->menu.right-l->menu.left,&bar)){
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
    if(PtInRect(&layout->system,point))return -2;
    if(!PtInRect(&layout->menu,point))return -3;
    origin.x=layout->menu.left;origin.y=layout->menu.top;
    if(!win16_style_bar_read_for_window(w,GetMenu(w),font,dpi,origin,layout->menu.right-layout->menu.left,&bar))return -3;
    hit=style_classic_menu_hit(bar.rects,bar.count,point);style_menu_bar_free(&bar);
    return hit<0?-3:hit;
}
typedef struct bar_measure {HFONT font;UINT dpi;} bar_measure;
static style_menu_bar_inclusion include_item(const MENUITEMINFOW *item,UINT *flags)
{
    if(item->fType&MFT_SEPARATOR)return STYLE_MENU_BAR_UNSUPPORTED;
    /* Old-UI MDI exposes system + restore; omit modern presentation cells
     * without editing the application's HMENU or its command routing. */
    if(item->hbmpItem==HBMMENU_MBAR_MINIMIZE || item->hbmpItem==HBMMENU_MBAR_MINIMIZE_D ||
       item->hbmpItem==HBMMENU_MBAR_CLOSE || item->hbmpItem==HBMMENU_MBAR_CLOSE_D)return STYLE_MENU_BAR_SKIP;
    if(item->hbmpItem && item->hbmpItem!=HBMMENU_SYSTEM && item->hbmpItem!=HBMMENU_MBAR_RESTORE)return STYLE_MENU_BAR_UNSUPPORTED;
    if(item->hbmpItem==HBMMENU_MBAR_RESTORE)*flags|=MFT_RIGHTJUSTIFY;
    return STYLE_MENU_BAR_INCLUDE;
}
static BOOL measure_item(void *context,HDC dc,const MENUITEMINFOW *item,SIZE *size)
{
    const bar_measure *measure=context;
    if(item->hbmpItem){*size=(SIZE){MulDiv(19,(int)measure->dpi,96),MulDiv(18,(int)measure->dpi,96)};return TRUE;}
    return win16_style_menu_measure(dc,measure->font,measure->dpi,item->dwTypeData,WIN16_STYLE_MENU_BAR,size);
}
BOOL win16_style_bar_read_for_window(HWND owner,HMENU menu,HFONT font,UINT dpi,POINT origin,int width,style_menu_bar_snapshot *bar)
{
    const style_menu_bar_profile profile={include_item,measure_item,FALSE};
    bar_measure measure={font,dpi};
    return style_menu_bar_read(owner,menu,origin,width,&profile,&measure,bar);
}
