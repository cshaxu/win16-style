#include "client_frame_input.h"
#include "lib-win32-style/application/menu/client_bar.h"
#include "lib-win32-style/application/menu/menu_input.h"
#include <commctrl.h>
#include <windowsx.h>

BOOL style_client_frame_input(HWND w,style_client_frame *s,UINT msg,WPARAM wp,LPARAM lp,const style_client_frame_input_services *v,LRESULT *result)
{
    style_full_client_layout l;const style_menu_bar_target menu_target=v->menu(w,s);
    if(style_menu_bar_input(&menu_target,msg,wp,lp))return TRUE;
    switch(msg){
    case WM_NCMOUSEMOVE:
        return !IsIconic(w);
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
        /* Minimized frames remain native; classic buttons have no hover art. */
        return !IsIconic(w);
    case WM_SYSCOMMAND:
        return (wp&0xfff0)==SC_KEYMENU && lp==L' ' && v->layout(w,s,&l) &&
            style_menu_bar_caption_key(&menu_target,IsRectEmpty(&l.buttons[CLASSIC_BUTTON_SYSTEM])?
                l.icon:l.buttons[CLASSIC_BUTTON_SYSTEM],wp,lp,result);
    case WM_NCLBUTTONDBLCLK:
        if(wp==HTMENU && v->layout(w,s,&l)){
            BOOL close=FALSE;
            (void)style_menu_client_item_screen(w,&l,s->fonts.menu,s->fonts.dpi,lp,&close);
            if(close)PostMessageW(w,WM_COMMAND,SC_CLOSE,0);
            return TRUE;
        }
        return FALSE;
    case WM_NCLBUTTONDOWN:
        if(style_caption_role((int)wp) && (wp!=HTSYSMENU || style_menu_bar_needs_system_button(w))){
            if((wp==HTHELP || wp==HTCLOSE) &&
               (!v->layout(w,s,&l) || IsRectEmpty(&l.buttons[style_caption_role((int)wp)])))return FALSE;
            if(style_caption_button_enabled(w,(int)wp)){
                const style_client_frame_button_view view=v->button_view(s);
                style_client_frame_button_press(w,&view,(int)wp);
            }
            return TRUE;
        }
        return FALSE;
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP: {
        const style_client_frame_button_view view=v->button_view(s);
        return style_client_frame_button_motion(w,&view,msg,lp);
    }
    case WM_CANCELMODE:
    case WM_CAPTURECHANGED: {
        const style_client_frame_button_view view=v->button_view(s);
        (void)style_client_frame_button_cancel(w,&view,msg);return FALSE;
    }
    default:return FALSE;
    }
}
