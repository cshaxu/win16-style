#include "frame_input.h"
#include "../menu/menu_bar.h"
#include "lib-win32-style/application/menu/menu_input.h"
#include <commctrl.h>
#include <windowsx.h>

BOOL win16_control_frame_input(HWND w,style_window *s,UINT msg,WPARAM wp,LPARAM lp,const win16_control_frame_input_services *v,LRESULT *result)
{
    win16_style_layout l;const style_menu_bar_target menu_target=v->menu(w,s);
    if(style_menu_bar_input(&menu_target,msg,wp,lp))return TRUE;
    switch(msg){
    case WM_NCMOUSEMOVE:
        return !IsIconic(w);
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
        /* Minimized frames remain native; classic buttons have no hover art. */
        return !IsIconic(w);
    case WM_SYSCOMMAND:
        if((wp&0xfff0)==SC_KEYMENU && lp==L' ' && v->layout(w,s,&l)){
            MENUBARINFO menu={.cbSize=sizeof(menu)};
            if((s->menu.bar.keyboard && s->menu.bar.selected==-2) ||
                (GetMenuBarInfo(w,OBJID_SYSMENU,0,&menu) && IsRectEmpty(&menu.rcBar))){
                RECT outer,button=l.system;GetWindowRect(w,&outer);OffsetRect(&button,outer.left,outer.top);
                *result=style_menu_bar_system(&menu_target,button,
                    s->menu.bar.keyboard && s->menu.bar.selected==STYLE_MENU_SYSTEM_ITEM,msg,wp,lp);
                return TRUE;
            }
        }
        return FALSE;
    case WM_NCLBUTTONDBLCLK:
        if(wp==HTMENU && v->layout(w,s,&l)){
            BOOL close=FALSE;
            (void)win16_control_menu_bar_item_screen(w,&l,s->fonts.menu,s->fonts.dpi,lp,&close);
            if(close)PostMessageW(w,WM_COMMAND,SC_CLOSE,0);
            return TRUE;
        }
        return FALSE;
    case WM_NCLBUTTONDOWN:
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
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP: {
        const win16_control_frame_button_view view=v->button_view(s);
        return win16_control_frame_button_motion(w,&view,msg,lp);
    }
    case WM_CANCELMODE:
    case WM_CAPTURECHANGED: {
        const win16_control_frame_button_view view=v->button_view(s);
        (void)win16_control_frame_button_cancel(w,&view,msg);return FALSE;
    }
    default:return FALSE;
    }
}
