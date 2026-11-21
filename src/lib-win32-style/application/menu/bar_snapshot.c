#include "bar_snapshot.h"
#include "classic_menu_layout.h"
#include "menu_input.h"
#include <stdlib.h>
#include <string.h>

void style_menu_bar_free(style_menu_bar_snapshot *bar)
{
    if(bar->items)for(UINT i=0;i<bar->count;++i)free(bar->items[i].text);
    free(bar->items);free(bar->rects);memset(bar,0,sizeof(*bar));
}
BOOL style_menu_bar_item_current(const style_menu_bar_snapshot *bar,UINT index)
{
    if(index>=bar->count)return FALSE;
    const style_menu_bar_item *item=&bar->items[index];
    MENUITEMINFOW current={.cbSize=sizeof(current),.fMask=MIIM_FTYPE|MIIM_DATA|MIIM_ID|MIIM_SUBMENU};
    return IsWindow(bar->owner) && GetMenu(bar->owner)==bar->menu &&
        GetMenuItemInfoW(bar->menu,item->position,TRUE,&current) && (current.fType&MFT_OWNERDRAW) &&
        current.dwItemData==item->data && current.wID==item->id && current.hSubMenu==item->popup;
}
BOOL style_menu_bar_read(HWND owner,HMENU menu,POINT origin,int width,
    const style_menu_bar_profile *profile,void *context,style_menu_bar_snapshot *bar)
{
    int count=GetMenuItemCount(menu);UINT n=0;SIZE *sizes=NULL;UINT *flags=NULL;HDC dc=NULL;BOOL ok=FALSE;
    memset(bar,0,sizeof(*bar));if(count<0 || (!profile->native_rects && width<=0))return FALSE;
    bar->owner=owner;bar->menu=menu;if(!count)return TRUE;
    bar->count=(UINT)count;
    bar->items=calloc(bar->count,sizeof(*bar->items));bar->rects=calloc(bar->count,sizeof(*bar->rects));
    if(!bar->items || !bar->rects)goto done;
    if(!profile->native_rects){
        sizes=calloc(bar->count,sizeof(*sizes));flags=calloc(bar->count,sizeof(*flags));dc=CreateCompatibleDC(NULL);
        if(!sizes || !flags || !dc)goto done;
    }
    for(UINT i=0;i<(UINT)count;++i){
        MENUITEMINFOW info={.cbSize=sizeof(info),.fMask=MIIM_STRING|MIIM_FTYPE|MIIM_STATE|MIIM_ID|MIIM_SUBMENU|MIIM_BITMAP|MIIM_DATA};
        if(!GetMenuItemInfoW(menu,i,TRUE,&info))goto done;
        UINT layout_flags=info.fType;
        style_menu_bar_inclusion included=profile->include?profile->include(context,&info,&layout_flags):STYLE_MENU_BAR_INCLUDE;
        if(included==STYLE_MENU_BAR_UNSUPPORTED)goto done;
        if(included==STYLE_MENU_BAR_SKIP)continue;
        BOOL app_draw=(info.fType&MFT_OWNERDRAW) && !info.hbmpItem;
        if(app_draw && (!owner || GetWindowThreadProcessId(owner,NULL)!=GetCurrentThreadId()))goto done;
        style_menu_bar_item *item=&bar->items[n];
        item->text=calloc((size_t)info.cch+1,sizeof(WCHAR));if(!item->text)goto done;
        info.dwTypeData=item->text;++info.cch;
        if(!GetMenuItemInfoW(menu,i,TRUE,&info))goto done;
        item->state=info.fState;item->id=info.wID;item->type=info.fType;item->popup=info.hSubMenu;
        item->control=info.hbmpItem;item->position=i;item->owner_draw=app_draw;item->data=info.dwItemData;
        if(profile->native_rects){
            if(!GetMenuItemRect(owner,menu,i,&bar->rects[n]))goto done;
        }else {
            flags[n]=layout_flags;
            if(app_draw){
                MEASUREITEMSTRUCT measure={.CtlType=ODT_MENU,.itemID=info.wID,.itemData=info.dwItemData};
                SendMessageW(owner,WM_MEASUREITEM,0,(LPARAM)&measure);
                if(!style_menu_bar_item_current(bar,n) || !measure.itemWidth || !measure.itemHeight ||
                   measure.itemWidth>32768 || measure.itemHeight>32768)goto done;
                sizes[n]=(SIZE){(LONG)measure.itemWidth,(LONG)measure.itemHeight};
            }else if(!profile->measure || !profile->measure(context,dc,&info,&sizes[n]))goto done;
        }
        ++n;
    }
    bar->count=n;
    if(profile->native_rects){
        RECT bounds={0};
        if(n){bounds=bar->rects[0];for(UINT i=1;i<n;++i)UnionRect(&bounds,&bounds,&bar->rects[i]);}
        bar->height=bounds.bottom-bounds.top;ok=TRUE;
    }else ok=style_classic_menu_arrange(origin,width,sizes,flags,n,bar->rects,&bar->height);
done: {
        DWORD outcome=GetLastError();
        if(dc)DeleteDC(dc);
        free(sizes);free(flags);if(!ok)style_menu_bar_free(bar);
        SetLastError(outcome);return ok;
    }
}
int style_menu_bar_mnemonic(const style_menu_bar_snapshot *bar,WCHAR key,BOOL stop_at_tab)
{
    for(UINT i=0;i<bar->count;++i)if(!(bar->items[i].state&MFS_DISABLED) &&
        style_menu_mnemonic(bar->items[i].text,key,stop_at_tab))return (int)i;
    return -1;
}
BOOL style_menu_bar_draw_owned(const style_menu_bar_snapshot *bar,UINT index,HDC dc)
{
    if(index>=bar->count)return FALSE;
    const style_menu_bar_item *item=&bar->items[index];UINT state=0;
    if(item->state&MFS_HILITE)state|=ODS_SELECTED;
    if(item->state&MFS_DISABLED)state|=ODS_DISABLED|ODS_GRAYED;
    if(item->state&MFS_CHECKED)state|=ODS_CHECKED;
    if(item->state&MFS_DEFAULT)state|=ODS_DEFAULT;
    if(SendMessageW(bar->owner,WM_QUERYUISTATE,0,0)&UISF_HIDEACCEL)state|=ODS_NOACCEL;
    if(GetAncestor(GetForegroundWindow(),GA_ROOT)!=GetAncestor(bar->owner,GA_ROOT))state|=ODS_INACTIVE;
    if(!style_menu_bar_item_current(bar,index))return FALSE;
    DRAWITEMSTRUCT draw={.CtlType=ODT_MENU,.itemID=item->id,.itemAction=ODA_DRAWENTIRE,.itemState=state,
        .hwndItem=(HWND)bar->menu,.hDC=dc,.rcItem=bar->rects[index],.itemData=item->data};
    int saved=SaveDC(dc);if(!saved)return TRUE;
    IntersectClipRect(dc,draw.rcItem.left,draw.rcItem.top,draw.rcItem.right,draw.rcItem.bottom);
    SendMessageW(bar->owner,WM_DRAWITEM,0,(LPARAM)&draw);RestoreDC(dc,saved);
    return IsWindow(bar->owner) && IsMenu(bar->menu);
}
