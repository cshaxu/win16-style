#include "menu_bar.h"
#include "lib-style/common/menu_input.h"
#include <stdlib.h>
#include <string.h>
#include <windowsx.h>

int win16_control_menu_bar_item_screen(HWND w,const win16_style_layout *l,HFONT font,UINT dpi,LPARAM lp,BOOL *close)
{
    win16_style_bar bar;POINT origin={l->menu.left,l->menu.top},point;RECT r;int item=-1;
    if(close)*close=FALSE;
    GetWindowRect(w,&r);point.x=GET_X_LPARAM(lp)-r.left;point.y=GET_Y_LPARAM(lp)-r.top;
    if(win16_style_bar_read_for_window(w,GetMenu(w),font,dpi,origin,l->menu.right-l->menu.left,&bar)){
        item=win16_style_menu_hit(bar.rects,bar.count,point);
        if(close && item>=0 && bar.items[item].control==HBMMENU_SYSTEM && bar.items[item].popup){
            UINT state=GetMenuState(bar.items[item].popup,SC_CLOSE,MF_BYCOMMAND);
            *close=state!=(UINT)-1 && !(state&(MF_DISABLED|MF_GRAYED));
        }
        win16_style_bar_free(&bar);
    }
    return item;
}
int win16_control_menu_bar_mnemonic(const win16_style_bar *bar,WCHAR key)
{
    int index=-1;
    for(UINT i=0;i<bar->count && index<0;++i){const WCHAR *text=bar->items[i].text;
        if(bar->items[i].state&MFS_DISABLED)continue;
        if(style_menu_mnemonic(text,key,TRUE))index=(int)i;
    }
    return index;
}
void win16_control_menu_input(MSG *m,const win16_control_menu_input_view *v)
{
    if((m->message==WM_KEYDOWN || m->message==WM_SYSKEYDOWN) &&
       m->wParam==VK_ESCAPE && *v->active==*v->root)*v->escape=TRUE;
    if(v->hit && (m->message==WM_LBUTTONDOWN || m->message==WM_NCLBUTTONDOWN ||
                  m->message==WM_MOUSEMOVE || m->message==WM_NCMOUSEMOVE)){
        POINT p={GET_X_LPARAM(m->lParam),GET_Y_LPARAM(m->lParam)};
        int hit=v->hit(v->hit_context,p);
        if(style_menu_pointer(m->message,hit,*v->selected,-3,-3,TRUE,v->target)){
            m->message=WM_NULL;m->wParam=0;m->lParam=0;EndMenu();}
    }
    if((m->message==WM_KEYDOWN || m->message==WM_SYSKEYDOWN) &&
       style_menu_bar_arrow(*v->window,*v->root,*v->active,(UINT)m->wParam,TRUE)){
        *v->direction=m->wParam==VK_LEFT?-1:1;
        m->message=WM_NULL;m->wParam=0;m->lParam=0;EndMenu();
    }
    if(m->hwnd==*v->window && (m->message==WM_NCLBUTTONDBLCLK || m->message==WM_LBUTTONDBLCLK)){
        POINT p={GET_X_LPARAM(m->lParam),GET_Y_LPARAM(m->lParam)};
        if(PtInRect(v->button,p)){
            *v->double_click=TRUE;*v->point=MAKELPARAM(p.x,p.y);
            m->message=WM_NULL;m->wParam=0;m->lParam=0;EndMenu();
        }
    }
}
void win16_control_menu_bar_leave(HWND w,const win16_control_bar_state_view *s,BOOL restore_focus)
{
    HWND prior=*s->focus;
    *s->keyboard=FALSE;*s->focus=NULL;*s->selected=-1;
    if(restore_focus && prior && IsWindow(prior) && GetFocus()==w)SetFocus(prior);
    if(!*s->detached)s->paint(w,s->context);
}
void win16_control_menu_bar_select(HWND w,const win16_control_bar_state_view *s,int direction)
{
    int count;win16_style_layout l;win16_style_bar bar;POINT origin;
    if(!s->layout(w,s->context,&l))return;
    origin.x=l.menu.left;origin.y=l.menu.top;
    if(!win16_style_bar_read_for_window(w,GetMenu(w),*s->font,*s->dpi,origin,l.menu.right-l.menu.left,&bar))return;
    count=(int)bar.count;win16_style_bar_free(&bar);
    if(count<=0)return;
    if(!*s->keyboard){*s->focus=GetFocus();*s->keyboard=TRUE;*s->selected=0;SetFocus(w);}
    else if(direction){
        BOOL system=(GetWindowLongPtrW(w,GWL_STYLE)&WS_SYSMENU)!=0;
        *s->selected=win16_control_menu_bar_next(*s->selected,direction,count,system);
    }
    if(!*s->detached)s->paint(w,s->context);
}
int win16_control_menu_bar_next(int selected,int direction,int count,BOOL system)
{
    if(count<=0 || !direction)return selected;
    if(selected==-2)return direction>0?0:count-1;
    int next=selected+direction;
    return (next<0 || next>=count)?(system?-2:(next+count)%count):next;
}
int win16_control_menu_bar_hit_screen(HWND w,const win16_style_layout *layout,
    HFONT font,UINT dpi,POINT point)
{
    win16_style_bar bar;RECT outer;POINT origin;int hit;
    if(!layout || !GetWindowRect(w,&outer))return -3;
    point.x-=outer.left;point.y-=outer.top;
    if(PtInRect(&layout->system,point))return -2;
    if(!PtInRect(&layout->menu,point))return -3;
    origin.x=layout->menu.left;origin.y=layout->menu.top;
    if(!win16_style_bar_read_for_window(w,GetMenu(w),font,dpi,origin,layout->menu.right-layout->menu.left,&bar))return -3;
    hit=win16_style_menu_hit(bar.rects,bar.count,point);win16_style_bar_free(&bar);
    return hit<0?-3:hit;
}
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
