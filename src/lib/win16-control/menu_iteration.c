#include "menu_iteration.h"
#include "menu_bar.h"

static int open_bar_once(HWND w,style_window *s,int index,int *hover,const win16_control_menu_bar_services *services)
{
    win16_style_layout l;win16_style_bar bar;POINT origin,anchor;RECT outer;
    HMENU popup;UINT command;BOOL merged_system;
    menu_frame frame={0};frame.previous=services->current();
    *hover=-3;frame.target=-3;
    if(index==-2){
        s->bar_direction=0;s->bar_hover=-3;
        SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');
        *hover=s->bar_hover;
        return s->bar_direction;
    }
    SendMessageW(w,WM_INITMENU,(WPARAM)GetMenu(w),0);
    if(s->detached || !services->layout(w,s,&l) || IsRectEmpty(&l.menu))return 0;
    origin.x=l.menu.left;origin.y=l.menu.top;
    if(!win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar))return 0;
    if(index<0 || (UINT)index>=bar.count || (bar.items[index].state&MFS_DISABLED)){
        win16_style_bar_free(&bar);return 0;}
    popup=bar.items[index].popup;command=bar.items[index].id;
    merged_system=bar.items[index].control==HBMMENU_SYSTEM;
    GetWindowRect(w,&outer);anchor.x=outer.left+bar.rects[index].left;
    anchor.y=outer.top+bar.rects[index].bottom;
    if(popup){
        frame.window=w;frame.root=frame.active=popup;
        frame.style=s;frame.selected=index;
        if(merged_system){frame.button=bar.rects[index];OffsetRect(&frame.button,outer.left,outer.top);}
        services->begin(&frame);
    }
    win16_style_bar_free(&bar);s->bar_selected=index;services->paint(w,s);
    if(s->bar_keyboard){
        if(popup)command=(UINT)TrackPopupMenuEx(popup,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
            anchor.x,anchor.y,w,NULL);
        if(!s->detached){
            if(command){services->leave(w,s,TRUE);if(!s->detached)PostMessageW(w,WM_COMMAND,command,0);}
            else if(!frame.direction && frame.target==-3 && !frame.escape)services->leave(w,s,TRUE);
            else services->paint(w,s); /* Keyboard Escape returns to the selected bar. */
        }
    }else{
        if(popup)TrackPopupMenuEx(popup,TPM_LEFTALIGN|TPM_TOPALIGN,anchor.x,anchor.y,w,NULL);
        else PostMessageW(w,WM_COMMAND,command,0);
        if(!s->detached){s->bar_selected=-1;services->paint(w,s);}
    }
    if(popup){
        services->end(&frame);
        if(frame.double_click && !s->detached)
            PostMessageW(w,WM_NCLBUTTONDBLCLK,HTMENU,frame.point);
    }
    *hover=frame.target;return frame.direction;
}


void win16_control_open_menu_bar(HWND w,style_window *s,int index,const win16_control_menu_bar_services *services)
{
    int direction,hover;BOOL mouse_origin=!s->bar_keyboard;
    HWND original_focus=s->bar_keyboard?s->bar_focus:GetFocus();
    ++s->calls; /* The iterator may change HWND; pin its current style state. */
    for(;;){
        win16_style_layout l;win16_style_bar bar;POINT origin;BOOL popup;int count=0;
        direction=open_bar_once(w,s,index,&hover,services);
        if(s->detached || (!direction && hover==-3))break;
        if(GetMenu(w) && services->layout(w,s,&l)){
            origin.x=l.menu.left;origin.y=l.menu.top;
            if(win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar)){
                count=(int)bar.count;win16_style_bar_free(&bar);}}
        if(direction && (index==-2 || (index==0 && direction<0) || (index==count-1 && direction>0))){
            MDINEXTMENU next={0};style_window *target;int next_index;
            next.hmenuIn=index==-2?GetSystemMenu(w,FALSE):GetMenu(w);
            SendMessageW(w,WM_NEXTMENU,direction<0?VK_LEFT:VK_RIGHT,(LPARAM)&next);
            if(s->detached)break;
            if(next.hwndNext && next.hmenuNext && IsWindow(next.hwndNext) && IsMenu(next.hmenuNext) &&
                GetWindowThreadProcessId(next.hwndNext,NULL)==GetCurrentThreadId() &&
                (target=services->lookup(next.hwndNext))!=NULL){
                if(next.hmenuNext==GetSystemMenu(next.hwndNext,FALSE))next_index=-2;
                else if(next.hmenuNext==GetMenu(next.hwndNext)){
                    if(!services->layout(next.hwndNext,target,&l))break;
                    origin.x=l.menu.left;origin.y=l.menu.top;
                    if(!win16_style_bar_read_for_window(next.hwndNext,next.hmenuNext,target->menu_font,target->dpi,origin,l.menu.right-l.menu.left,&bar))break;
                    next_index=direction>0?0:(int)bar.count-1;win16_style_bar_free(&bar);
                    if(next_index<0)break;
                }else break;
                ++target->calls;
                services->leave(w,s,FALSE);
                if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
                w=next.hwndNext;s=target;index=next_index;
                if(s->detached)break;
                SetFocus(w);
                if(s->detached)break;
                s->bar_focus=original_focus;s->bar_keyboard=TRUE;s->bar_selected=index;
                services->paint(w,s);continue;
            }
        }
        /* Switch selection, but never execute a direct bar command merely
         * because an arrow reached it. Enter still owns command invocation. */
        if(!s->bar_keyboard)services->select(w,s,0);
        if(hover!=-3){s->bar_selected=hover;services->paint(w,s);}
        else {s->bar_selected=index;services->select(w,s,direction);}
        index=s->bar_selected;
        if(index==-2)continue;
        if(!services->layout(w,s,&l))break;
        origin.x=l.menu.left;
        origin.y=l.menu.top;
        if(!win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar))break;
        popup=index>=0 && (UINT)index<bar.count && bar.items[index].popup!=NULL;
        win16_style_bar_free(&bar);if(!popup)break;
    }
    /* Switching a mouse-opened popup uses the keyboard iterator internally,
     * but must not leave Alt/F10 mode armed after cancellation. */
    if(mouse_origin && !s->detached)services->leave(w,s,TRUE);
    if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
}
