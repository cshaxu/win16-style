#include "menu_bar.h"
#include <stdlib.h>
#include <string.h>

void win16_style_bar_free(win16_style_bar *bar)
{
    UINT i;
    if(bar->items)for(i=0;i<bar->count;++i)free(bar->items[i].text);
    free(bar->items);free(bar->rects);memset(bar,0,sizeof(*bar));
}
BOOL win16_style_bar_read_for_window(HWND owner,HMENU menu,HFONT font,UINT dpi,POINT origin,int width,win16_style_bar *bar)
{
    int count=GetMenuItemCount(menu);UINT i,n=0;SIZE *sizes=NULL;UINT *flags=NULL;HDC dc=NULL;BOOL ok=FALSE;
    memset(bar,0,sizeof(*bar));if(count<0 || width<=0)return FALSE;
    bar->owner=owner;bar->menu=menu;
    if(!count)return TRUE;
    bar->count=(UINT)count;
    bar->items=calloc(bar->count,sizeof(*bar->items));bar->rects=calloc(bar->count,sizeof(*bar->rects));
    sizes=calloc(bar->count,sizeof(*sizes));flags=calloc(bar->count,sizeof(*flags));
    dc=CreateCompatibleDC(NULL);
    if(!bar->items || !bar->rects || !sizes || !flags || !dc)goto done;
    for(i=0;i<bar->count;++i){
        MENUITEMINFOW info={.cbSize=sizeof(info)};
        info.fMask=MIIM_STRING|MIIM_FTYPE|MIIM_STATE|MIIM_ID|MIIM_SUBMENU|MIIM_BITMAP|MIIM_DATA;
        if(!GetMenuItemInfoW(menu,i,TRUE,&info) || (info.fType&MFT_SEPARATOR))goto done;
        /* NTUSER's old-UI MDI merge has system + restore only. Modern USER
         * adds minimize/close; omit those presentation cells without editing
         * the application's actual HMENU or its native command routing. */
        if(info.hbmpItem==HBMMENU_MBAR_MINIMIZE || info.hbmpItem==HBMMENU_MBAR_MINIMIZE_D ||
            info.hbmpItem==HBMMENU_MBAR_CLOSE || info.hbmpItem==HBMMENU_MBAR_CLOSE_D)continue;
        if(info.hbmpItem && info.hbmpItem!=HBMMENU_SYSTEM && info.hbmpItem!=HBMMENU_MBAR_RESTORE)goto done;
        /* USER marks its merged restore item owner-drawn as well as giving it
         * the standard bitmap identity. Other owner-draw entries require an
         * explicit same-thread owner and keep the application callbacks. */
        BOOL app_draw=(info.fType&MFT_OWNERDRAW) && !info.hbmpItem;
        if(app_draw && (!owner || GetWindowThreadProcessId(owner,NULL)!=GetCurrentThreadId()))goto done;
        bar->items[n].text=calloc((size_t)info.cch+1,sizeof(WCHAR));
        if(!bar->items[n].text)goto done;
        info.dwTypeData=bar->items[n].text;++info.cch;
        if(!GetMenuItemInfoW(menu,i,TRUE,&info))goto done;
        bar->items[n].state=info.fState;bar->items[n].id=info.wID;bar->items[n].popup=info.hSubMenu;
        bar->items[n].control=info.hbmpItem;flags[n]=info.fType;
        bar->items[n].position=i;bar->items[n].owner_draw=app_draw;bar->items[n].data=info.dwItemData;
        if(app_draw){
            MEASUREITEMSTRUCT measure={.CtlType=ODT_MENU,.itemID=info.wID,.itemData=info.dwItemData};
            SendMessageW(owner,WM_MEASUREITEM,0,(LPARAM)&measure);
            if(!win16_style_bar_item_current(bar,n) || !measure.itemWidth || !measure.itemHeight ||
               measure.itemWidth>32768 || measure.itemHeight>32768)goto done;
            sizes[n].cx=(LONG)measure.itemWidth;sizes[n].cy=(LONG)measure.itemHeight;
        }
        else if(info.hbmpItem){sizes[n].cx=MulDiv(19,(int)dpi,96);sizes[n].cy=MulDiv(18,(int)dpi,96);
            if(info.hbmpItem==HBMMENU_MBAR_RESTORE)flags[n]|=MFT_RIGHTJUSTIFY;}
        else if(!win16_style_menu_measure(dc,font,dpi,info.dwTypeData,WIN16_STYLE_MENU_BAR,&sizes[n]))goto done;
        ++n;
    }
    bar->count=n;
    ok=win16_style_menu_arrange(origin,width,sizes,flags,bar->count,bar->rects,&bar->height);
done:
    if(dc)DeleteDC(dc);
    free(sizes);
    free(flags);
    if(!ok)win16_style_bar_free(bar);
    return ok;
}

BOOL win16_style_bar_read(HMENU menu,HFONT font,UINT dpi,POINT origin,int width,win16_style_bar *bar)
{return win16_style_bar_read_for_window(NULL,menu,font,dpi,origin,width,bar);}

BOOL win16_style_bar_item_current(const win16_style_bar *bar,UINT index)
{
    const win16_style_bar_item *item=&bar->items[index];
    MENUITEMINFOW current={.cbSize=sizeof(current),.fMask=MIIM_FTYPE|MIIM_DATA|MIIM_ID|MIIM_SUBMENU};
    return IsWindow(bar->owner) && GetMenu(bar->owner)==bar->menu &&
        GetMenuItemInfoW(bar->menu,item->position,TRUE,&current) &&
        (current.fType&MFT_OWNERDRAW) && current.dwItemData==item->data &&
        current.wID==item->id && current.hSubMenu==item->popup;
}
