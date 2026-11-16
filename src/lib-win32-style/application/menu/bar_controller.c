#include "bar_controller.h"
#include "menu_input.h"
#include <windowsx.h>

static BOOL live(const style_menu_bar_target *t)
{return !t->instance->detached && IsWindow(t->window);}
static int hit(const style_menu_tracking *tracking,POINT point)
{const style_menu_bar_target *t=tracking->context;return live(t)?t->ops->hit(t,point):STYLE_MENU_NO_ITEM;}
void style_menu_bar_finish(const style_menu_bar_target *t,BOOL restore)
{
    style_menu_bar_leave(t->owner,t->window,restore);
    if(live(t))t->ops->paint(t);
}
BOOL style_menu_bar_choose(const style_menu_bar_target *t,int direction)
{
    style_menu_bar_snapshot bar;if(!live(t) || !t->ops->read(t,&bar))return FALSE;
    int count=(int)bar.count;style_menu_bar_free(&bar);if(count<=0 || !live(t))return FALSE;
    int selected=0;
    if(t->owner->bar.keyboard){
        if(t->ops->navigation==STYLE_MENU_TRACK_NATIVE_POPUPS)
            selected=style_menu_next_enabled(GetMenu(t->window),t->owner->bar.selected,direction,FALSE);
        else selected=style_menu_bar_step(t->owner->bar.selected,direction,count,
            (GetWindowLongPtrW(t->window,GWL_STYLE)&WS_SYSMENU)!=0);
    }
    if(selected<0 && selected!=STYLE_MENU_SYSTEM_ITEM)return FALSE;
    if(!style_menu_bar_select(t->owner,t->window,selected) || !live(t))return FALSE;
    t->ops->paint(t);return TRUE;
}
static int open_once(style_menu_bar_target *t,int index,style_menu_tracking *tracking)
{
    HWND w=t->window;style_menu_bar_state *state=&t->owner->bar;
    BOOL native=t->ops->navigation==STYLE_MENU_TRACK_NATIVE_POPUPS;
    tracking->target=STYLE_MENU_NO_ITEM;tracking->direction=0;
    if(index==STYLE_MENU_SYSTEM_ITEM){
        state->direction=0;state->hover=STYLE_MENU_NO_ITEM;
        SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');
        tracking->target=state->hover;return state->direction;
    }
    HMENU menu=GetMenu(w);SendMessageW(w,WM_INITMENU,(WPARAM)menu,0);
    if(!live(t) || (native && GetMenu(w)!=menu))return 0;
    style_menu_bar_snapshot bar;if(!t->ops->read(t,&bar))return 0;
    if(!live(t) || index<0 || (UINT)index>=bar.count || (bar.items[index].state&MFS_DISABLED)){
        style_menu_bar_free(&bar);return 0;
    }
    HMENU popup=bar.items[index].popup;UINT command=bar.items[index].id;
    POINT anchor={bar.rects[index].left,bar.rects[index].bottom};
    tracking->window=w;tracking->context=t;tracking->hit=hit;tracking->selected=index;
    tracking->root=tracking->active=popup;
    if(bar.items[index].control==HBMMENU_SYSTEM)tracking->button=bar.rects[index];
    BOOL linked=!native && popup && style_menu_tracking_begin(tracking,WH_GETMESSAGE,style_menu_tracking_input);
    style_menu_bar_free(&bar);
    if(native)state->highlighted=index;else state->selected=index;
    t->ops->paint(t);
    if(!native && state->keyboard){
        if(popup)command=(UINT)TrackPopupMenuEx(popup,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,anchor.x,anchor.y,w,NULL);
        if(live(t)){
            if(command){style_menu_bar_finish(t,TRUE);if(live(t))PostMessageW(w,WM_COMMAND,command,0);}
            else if(!tracking->direction && tracking->target==STYLE_MENU_NO_ITEM && !tracking->escape)style_menu_bar_finish(t,TRUE);
            else t->ops->paint(t);
        }
    }else {
        if(popup)TrackPopupMenuEx(popup,TPM_LEFTALIGN|TPM_TOPALIGN,anchor.x,anchor.y,w,NULL);
        else PostMessageW(w,WM_COMMAND,command,0);
        if(live(t)){
            if(native)state->highlighted=-1;else state->selected=-1;
            t->ops->paint(t);
        }
    }
    if(linked)style_menu_tracking_end(tracking);
    if(!native && popup && tracking->double_click && live(t))PostMessageW(w,WM_NCLBUTTONDBLCLK,HTMENU,tracking->point);
    return tracking->direction;
}
void style_menu_bar_open(style_menu_bar_target t,int index)
{
    BOOL native=t.ops->navigation==STYLE_MENU_TRACK_NATIVE_POPUPS,mouse_origin=!t.owner->bar.keyboard;
    HWND original_focus=t.owner->bar.keyboard?t.owner->bar.focus:GetFocus();
    style_instance_retain(t.instance);
    style_menu_tracking tracking={.window=t.window,.selected=index,.target=STYLE_MENU_NO_ITEM,
        .armed=native?(GetKeyState(VK_LBUTTON)&0x8000)==0:TRUE,.hit=hit,.context=&t,.navigation=t.ops->navigation};
    BOOL linked=native && style_menu_tracking_begin(&tracking,WH_MSGFILTER,style_menu_tracking_input);
    for(;;){
        if(!native){tracking.escape=FALSE;tracking.double_click=FALSE;tracking.button=(RECT){0};}
        int direction=open_once(&t,index,&tracking),hover=tracking.target;
        if(!live(&t) || (!direction && hover==STYLE_MENU_NO_ITEM))break;
        if(native){if(hover<0)break;index=hover;continue;}
        style_menu_bar_snapshot bar;int count=0;
        if(t.ops->read(&t,&bar)){count=(int)bar.count;style_menu_bar_free(&bar);}
        if(direction && (index==STYLE_MENU_SYSTEM_ITEM || (!index && direction<0) || (index==count-1 && direction>0))){
            MDINEXTMENU next={.hmenuIn=index==STYLE_MENU_SYSTEM_ITEM?GetSystemMenu(t.window,FALSE):GetMenu(t.window)};
            SendMessageW(t.window,WM_NEXTMENU,direction<0?VK_LEFT:VK_RIGHT,(LPARAM)&next);
            if(!live(&t))break;
            style_menu_bar_target target;
            if(next.hwndNext && next.hmenuNext && IsWindow(next.hwndNext) && IsMenu(next.hmenuNext) &&
               GetWindowThreadProcessId(next.hwndNext,NULL)==GetCurrentThreadId() && t.ops->lookup && t.ops->lookup(next.hwndNext,&target)){
                style_instance_retain(target.instance);
                int selected=-1;
                if(next.hmenuNext==GetSystemMenu(next.hwndNext,FALSE))selected=STYLE_MENU_SYSTEM_ITEM;
                else if(next.hmenuNext==GetMenu(next.hwndNext)){
                    if(target.ops->read(&target,&bar)){
                        selected=direction>0?0:(int)bar.count-1;style_menu_bar_free(&bar);
                    }
                }
                if((selected<0 && selected!=STYLE_MENU_SYSTEM_ITEM) || !live(&target)){
                    style_instance_release(target.instance);break;
                }
                style_menu_bar_finish(&t,FALSE);style_instance_release(t.instance);
                t=target;index=selected;if(!live(&t))break;
                SetFocus(t.window);if(!live(&t))break;
                t.owner->bar.focus=original_focus;t.owner->bar.keyboard=TRUE;t.owner->bar.selected=index;
                t.ops->paint(&t);continue;
            }
        }
        /* An arrow selects direct commands; only Enter invokes them. */
        if(!t.owner->bar.keyboard)style_menu_bar_choose(&t,0);
        if(!live(&t))break;
        if(hover!=STYLE_MENU_NO_ITEM){t.owner->bar.selected=hover;t.ops->paint(&t);}
        else {t.owner->bar.selected=index;style_menu_bar_choose(&t,direction);}
        index=t.owner->bar.selected;if(index==STYLE_MENU_SYSTEM_ITEM)continue;
        if(!t.ops->read(&t,&bar))break;
        BOOL popup=index>=0 && (UINT)index<bar.count && bar.items[index].popup;
        style_menu_bar_free(&bar);if(!popup)break;
    }
    if(linked)style_menu_tracking_end(&tracking);
    if(native)t.owner->bar.highlighted=-1;
    else if(mouse_origin && live(&t))style_menu_bar_finish(&t,TRUE);
    style_instance_release(t.instance);
}

BOOL style_menu_bar_input(const style_menu_bar_target *t,UINT message,WPARAM wp,LPARAM lp)
{
    switch(message){
    case WM_KILLFOCUS:case WM_SYSCOMMAND:case WM_KEYDOWN:case WM_SYSKEYDOWN:
    case WM_NCLBUTTONDOWN:case WM_NCMOUSEMOVE:case WM_LBUTTONDOWN:case WM_CANCELMODE:break;
    default:return FALSE;
    }
    if(!live(t))return FALSE;
    BOOL native=t->ops->navigation==STYLE_MENU_TRACK_NATIVE_POPUPS;
    style_menu_bar_state *state=&t->owner->bar;
    if(!GetMenu(t->window))return FALSE;
    if(native){
        style_menu_bar_snapshot bar;if(!t->ops->read(t,&bar))return FALSE;
        style_menu_bar_free(&bar);
    }
    if(message==WM_KILLFOCUS){if(state->keyboard)style_menu_bar_finish(t,FALSE);return FALSE;}
    if(message==WM_SYSCOMMAND && (wp&0xfff0)==SC_KEYMENU){
        if(!lp){
            if(state->keyboard)style_menu_bar_finish(t,TRUE);else style_menu_bar_choose(t,0);
            return TRUE;
        }
        if(lp!=L' '){
            style_menu_bar_snapshot bar;int index=-1;
            if(t->ops->read(t,&bar)){index=style_menu_bar_mnemonic(&bar,(WCHAR)lp,!native);style_menu_bar_free(&bar);}
            if(index>=0){
                if(!native && !state->keyboard)style_menu_bar_choose(t,0);
                if(live(t))style_menu_bar_open(*t,index);
            }
            return !native || index>=0;
        }
    }
    if(state->keyboard && (message==WM_KEYDOWN || (native && message==WM_SYSKEYDOWN))){
        style_menu_key_action action=style_menu_key(wp);
        if(action==STYLE_MENU_KEY_LEAVE){style_menu_bar_finish(t,TRUE);return TRUE;}
        if(action==STYLE_MENU_KEY_PREVIOUS || action==STYLE_MENU_KEY_NEXT){
            style_menu_bar_choose(t,action==STYLE_MENU_KEY_PREVIOUS?-1:1);return TRUE;
        }
        if(action==STYLE_MENU_KEY_OPEN){
            int selected=state->selected;style_menu_bar_open(*t,selected);
            if(native && live(t) && state->keyboard){state->highlighted=selected;t->ops->paint(t);}
            return TRUE;
        }
    }
    if(message==WM_NCLBUTTONDOWN){
        if(state->keyboard && (native || !style_menu_tracking_current()))style_menu_bar_finish(t,!native);
        if(wp==HTMENU){
            POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};int index=t->ops->hit(t,point);
            if(index>=0 && live(t))style_menu_bar_open(*t,index);
            return TRUE;
        }
    }
    if(!native && message==WM_NCMOUSEMOVE && state->keyboard && !style_menu_tracking_current()){
        POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};int index=t->ops->hit(t,point);
        if(index!=STYLE_MENU_NO_ITEM && index!=state->selected){
            state->selected=index;t->ops->paint(t);BOOL popup=index==STYLE_MENU_SYSTEM_ITEM;
            style_menu_bar_snapshot bar;
            if(index>=0 && t->ops->read(t,&bar)){
                popup=(UINT)index<bar.count && bar.items[index].popup;style_menu_bar_free(&bar);
            }
            if(popup && live(t))style_menu_bar_open(*t,index);
        }
    }
    if(!native && message==WM_LBUTTONDOWN && state->keyboard && !style_menu_tracking_current())style_menu_bar_finish(t,TRUE);
    if(!native && message==WM_CANCELMODE){
        style_menu_tracking *tracking=style_menu_tracking_current();
        if(tracking && tracking->window==t->window)EndMenu();
        if(state->keyboard)style_menu_bar_finish(t,TRUE);
    }
    return FALSE;
}
LRESULT style_menu_bar_system(const style_menu_bar_target *t,RECT button,BOOL navigate,UINT message,WPARAM wp,LPARAM lp)
{
    style_menu_tracking tracking={.window=t->window,.button=button,.armed=TRUE};
    if(navigate){
        tracking.root=tracking.active=GetSystemMenu(t->window,FALSE);
        tracking.context=(void *)t;tracking.hit=hit;tracking.selected=STYLE_MENU_SYSTEM_ITEM;tracking.target=STYLE_MENU_NO_ITEM;
    }
    BOOL linked=style_menu_tracking_begin(&tracking,WH_GETMESSAGE,style_menu_tracking_input);
    LRESULT result=DefSubclassProc(t->window,message,wp,lp);
    if(linked)style_menu_tracking_end(&tracking);
    if(tracking.root){
        t->owner->bar.direction=tracking.direction;t->owner->bar.hover=tracking.target;
        if(live(t) && !tracking.direction && tracking.target==STYLE_MENU_NO_ITEM && !tracking.escape)style_menu_bar_finish(t,TRUE);
    }
    if(tracking.double_click && live(t))PostMessageW(t->window,WM_NCLBUTTONDBLCLK,HTSYSMENU,tracking.point);
    return result;
}
