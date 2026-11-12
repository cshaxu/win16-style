#include "frame_input.h"
#include "menu_bar.h"
#include "lib-style/common/menu_input.h"
#include <commctrl.h>
#include <windowsx.h>

BOOL win16_control_frame_input(HWND w,style_window *s,UINT msg,WPARAM wp,LPARAM lp,const win16_control_frame_input_services *v,LRESULT *result)
{
    win16_style_layout l;
    switch(msg){
    case WM_KEYDOWN:
        if(s->bar_keyboard){
            style_menu_key_action action=style_menu_key(wp);
            if(action==STYLE_MENU_KEY_PREVIOUS || action==STYLE_MENU_KEY_NEXT){v->select(w,s,action==STYLE_MENU_KEY_PREVIOUS?-1:1);return TRUE;}
            if(action==STYLE_MENU_KEY_LEAVE){v->leave(w,s,TRUE);return TRUE;}
            if(action==STYLE_MENU_KEY_OPEN){v->open(w,s,s->bar_selected);return TRUE;}
        }
        return FALSE;
    case WM_KILLFOCUS:
        if(s->bar_keyboard)v->leave(w,s,FALSE);
        return FALSE;
    case WM_NCMOUSEMOVE:
        if(s->bar_keyboard && !v->current()){
            POINT cursor={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};int hit=v->bar_hit(w,s,cursor);
            if(hit!=-3 && hit!=s->bar_selected){
                win16_style_bar bar;POINT origin;BOOL popup=hit==-2;
                s->bar_selected=hit;v->paint(w,s);
                if(hit>=0 && v->layout(w,s,&l)){origin.x=l.menu.left;origin.y=l.menu.top;
                    if(win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar)){
                        popup=(UINT)hit<bar.count && bar.items[hit].popup!=NULL;
                        win16_style_bar_free(&bar);}}
                if(popup)v->open(w,s,hit);
            }
        }
        return !IsIconic(w);
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
        /* Minimized frames remain native; classic buttons have no hover art. */
        return !IsIconic(w);
    case WM_SYSCOMMAND:
        if((wp&0xfff0)==SC_KEYMENU && !lp && GetMenu(w)){
            if(s->bar_keyboard)v->leave(w,s,TRUE);else v->select(w,s,0);
            return TRUE;
        }
        if((wp&0xfff0)==SC_KEYMENU && lp && lp!=L' ' && GetMenu(w) && v->layout(w,s,&l)){
            win16_style_bar bar;POINT origin={l.menu.left,l.menu.top};int index=-1;
            if(win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar)){
                index=win16_control_menu_bar_mnemonic(&bar,(WCHAR)lp);
                win16_style_bar_free(&bar);
            }
            if(index>=0){
                if(!s->bar_keyboard)v->select(w,s,0);
                if(!s->detached)v->open(w,s,index);
            }
            return TRUE;
        }
        if((wp&0xfff0)==SC_KEYMENU && lp==L' ' && v->layout(w,s,&l)){
            MENUBARINFO menu={.cbSize=sizeof(menu)};
            if((s->bar_keyboard && s->bar_selected==-2) ||
                (GetMenuBarInfo(w,OBJID_SYSMENU,0,&menu) && IsRectEmpty(&menu.rcBar))){
                menu_frame frame={0};RECT r;frame.previous=v->current();
                GetWindowRect(w,&r);frame.window=w;frame.button=l.system;
                if(s->bar_keyboard && s->bar_selected==-2){
                    frame.root=frame.active=GetSystemMenu(w,FALSE);
                    frame.style=s;frame.selected=-2;frame.target=-3;
                }
                OffsetRect(&frame.button,r.left,r.top);v->begin(&frame);
                *result=DefSubclassProc(w,msg,wp,lp);v->end(&frame);
                if(frame.root){
                    s->bar_direction=frame.direction;s->bar_hover=frame.target;
                    if(!s->detached && !frame.direction && frame.target==-3 && !frame.escape)
                        v->leave(w,s,TRUE);
                }
                if(frame.double_click && !s->detached)
                    PostMessageW(w,WM_NCLBUTTONDBLCLK,HTSYSMENU,frame.point);
                return TRUE;
            }
        }
        return FALSE;
    case WM_NCLBUTTONDBLCLK:
        if(wp==HTMENU && v->layout(w,s,&l)){
            BOOL close=FALSE;
            (void)win16_control_menu_bar_item_screen(w,&l,s->menu_font,s->dpi,lp,&close);
            if(close)PostMessageW(w,WM_COMMAND,SC_CLOSE,0);
            return TRUE;
        }
        return FALSE;
    case WM_NCLBUTTONDOWN:
        if(s->bar_keyboard && !v->current())v->leave(w,s,TRUE);
        if(wp==HTMENU && v->layout(w,s,&l)){
            int item=win16_control_menu_bar_item_screen(w,&l,s->menu_font,s->dpi,lp,NULL);
            if(item>=0 && !s->detached)v->open(w,s,item);
            return TRUE;
        }
        if(wp==HTSYSMENU){
            MENUBARINFO menu={.cbSize=sizeof(menu)};
            if(GetMenuBarInfo(w,OBJID_SYSMENU,0,&menu) && IsRectEmpty(&menu.rcBar)){
                const win16_control_frame_button_view view=v->button_view(s);
                win16_control_frame_button_press(w,&view,HTSYSMENU);return TRUE;
            }
        }
        if(wp==HTMINBUTTON || wp==HTMAXBUTTON){
            const win16_control_frame_button_view view=v->button_view(s);
            win16_control_frame_button_press(w,&view,(int)wp);return TRUE;
        }
        return FALSE;
    case WM_LBUTTONDOWN:
        if(s->bar_keyboard && !v->current())v->leave(w,s,TRUE);
        return FALSE;
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP: {
        const win16_control_frame_button_view view=v->button_view(s);
        return win16_control_frame_button_motion(w,&view,msg,lp);
    }
    case WM_CANCELMODE:
    case WM_CAPTURECHANGED: {
        menu_frame *tracking=v->current();
        if(msg==WM_CANCELMODE && tracking && tracking->window==w)EndMenu();
        if(msg==WM_CANCELMODE && s->bar_keyboard)v->leave(w,s,TRUE);
        const win16_control_frame_button_view view=v->button_view(s);
        (void)win16_control_frame_button_cancel(w,&view,msg);return FALSE;
    }
    default:return FALSE;
    }
}
