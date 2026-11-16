#include "owner.h"
#include "tracking.h"

BOOL style_menu_bar_select(style_menu_owner *s,HWND window,int selected)
{
    if(s->detached)return FALSE;
    s->bar.selected=s->bar.highlighted=selected;
    if(!s->bar.keyboard){s->bar.focus=GetFocus();s->bar.keyboard=TRUE;SetFocus(window);}
    return !s->detached;
}
void style_menu_bar_leave(style_menu_owner *s,HWND window,BOOL restore)
{
    HWND focus=s->bar.focus;
    s->bar.keyboard=FALSE;s->bar.focus=NULL;s->bar.selected=s->bar.highlighted=-1;
    if(restore && focus && IsWindow(focus) && GetFocus()==window)SetFocus(focus);
}
int style_menu_bar_step(int selected,int direction,int count,BOOL system)
{
    if(count<=0 || !direction)return selected;
    if(selected==STYLE_MENU_SYSTEM_ITEM)return direction>0?0:count-1;
    int next=selected+direction;
    return next<0 || next>=count?(system?STYLE_MENU_SYSTEM_ITEM:(next+count)%count):next;
}
