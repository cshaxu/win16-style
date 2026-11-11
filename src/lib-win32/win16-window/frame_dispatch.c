#include "frame_dispatch.h"
#include "frame_layout.h"
#include "lib-win32/style-common/menu_hit.h"
#include <commctrl.h>
#include <windowsx.h>
static BOOL queue_repaint(HWND,style_window *);
static LRESULT CALLBACK repaint_proc(HWND timer,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    style_window *s=(style_window *)data;
    (void)id;
    if(msg==WM_TIMER && wp==(UINT_PTR)s){
        KillTimer(timer,(UINT_PTR)s);s->xaml_repaint_pending=FALSE;
        ++s->calls;
        if(s->xaml_waiting){
            HWND owner=s->xaml_repaint_owner;
            if(!win16_control_frame_xaml_bridge(owner,TRUE) && GetTickCount64()<s->xaml_deadline && queue_repaint(owner,s)){
                if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
                return 0;
            }
            s->xaml_waiting=FALSE;s->pending_frame=FALSE;
            RECT client;BOOL normal=!IsIconic(owner)&&!IsZoomed(owner);
            win16_control_client_screen(owner,&client);s->xaml_activate(owner,s,&client,normal);
        }else{
            s->xaml_repainting=TRUE;
            SendMessageW(s->xaml_repaint_owner,WM_NCPAINT,1,0);
            s->xaml_repainting=FALSE;
        }
        if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
        return 0;
    }
    return DefSubclassProc(timer,msg,wp,lp);
}
void win16_control_frame_cancel_repaint(style_window *s)
{
    if(s->xaml_repaint_window){
        KillTimer(s->xaml_repaint_window,(UINT_PTR)s);
        DestroyWindow(s->xaml_repaint_window);
        s->xaml_repaint_window=NULL;s->xaml_repaint_pending=FALSE;s->xaml_waiting=FALSE;
    }
}
static BOOL queue_repaint(HWND w,style_window *s)
{
    if(!s->xaml_repaint_window){
        HWND timer=CreateWindowExW(0,L"STATIC",L"",0,0,0,0,0,HWND_MESSAGE,NULL,GetModuleHandleW(NULL),NULL);
        if(!timer)return FALSE;
        if(s->detached || !IsWindow(w)){DestroyWindow(timer);return FALSE;}
        if(!SetWindowSubclass(timer,repaint_proc,(UINT_PTR)repaint_proc,(DWORD_PTR)s)){DestroyWindow(timer);return FALSE;}
        if(s->detached || !IsWindow(w)){DestroyWindow(timer);return FALSE;}
        s->xaml_repaint_owner=w;s->xaml_repaint_window=timer;
    }
    if(!SetTimer(s->xaml_repaint_window,(UINT_PTR)s,30,NULL))return FALSE;
    s->xaml_repaint_pending=TRUE;return TRUE;
}
LRESULT win16_control_frame_dispatch(HWND w,style_window *s,UINT msg,WPARAM wp,LPARAM lp,
    const win16_control_frame_dispatch_services *v)
{

    LRESULT result=0;win16_style_layout l;POINT point;RECT r;
    ++s->calls;
    if(s->menu_only && msg!=WM_ENTERMENULOOP && msg!=WM_EXITMENULOOP && msg!=WM_INITMENUPOPUP && msg!=WM_MENUCHAR && msg!=WM_MEASUREITEM && msg!=WM_DRAWITEM && msg!=WM_NCDESTROY)goto forward;
    /* Maximized MDI children merge their controls into the frame menu.
     * Preserve DefMDIChildProc's native geometry/dispatch for that state;
     * merged-control retro painting is a separate S4 menu surface. */
    if((GetWindowLongPtr(w,GWL_EXSTYLE)&WS_EX_MDICHILD) && IsZoomed(w) &&
       msg!=WM_NCDESTROY)goto forward;
    /* Hidden creation must retain the application's geometry until showing.
     * XAML bridges may become visible after the parent show notification. */
    if(s->pending_frame && msg==WM_NCACTIVATE)s->active=(BOOL)wp;
    if(s->pending_frame && msg==WM_DPICHANGED){
        result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached)v->message->refresh_font(w,s);
        goto done;
    }
    if(s->pending_frame && msg!=WM_SHOWWINDOW && msg!=WM_NCDESTROY)goto forward;
    if(msg==WM_SHOWWINDOW && s->pending_frame){
        if(s->xaml_waiting){if(!wp)win16_control_frame_cancel_repaint(s);goto forward;}
        if(wp && win16_control_frame_xaml_bridge(w,FALSE)){
            s->xaml_waiting=TRUE;s->xaml_deadline=GetTickCount64()+2000;s->xaml_activate=v->message->activate;
            if(queue_repaint(w,s))goto forward;
            s->xaml_waiting=FALSE;
            if(s->detached)goto done;
        }
    }
    /* Our menu bar is custom nonclient chrome. USER hot-tracks HTMENU before
     * NC mouse dispatch (classic/WOW has no UAH draw callback to intercept).
     * Expose neutral nonclient hit-test ownership and route clicks by our
     * measured menu rectangle; never alter the application's HMENU. */
    if((msg==WM_NCLBUTTONDOWN || msg==WM_NCLBUTTONDBLCLK) && wp==HTBORDER && v->input->layout(w,s,&l)){
        GetWindowRect(w,&r);point.x=GET_X_LPARAM(lp)-r.left;point.y=GET_Y_LPARAM(lp)-r.top;
        wp=style_menu_route_click(msg,wp,PtInRect(&l.menu,point));
    }
    switch(msg){
    case WM_GETICON:
    case WM_SETICON: {
        LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
        if(!s->xaml_icon_update && !(style&WS_CHILD) && (style&WS_DLGFRAME) &&
           win16_control_frame_xaml_bridge(w,FALSE)){
            s->xaml_icon_update=TRUE;SetLastError(0);
            LONG_PTR old=SetWindowLongPtrW(w,GWL_STYLE,style&~WS_DLGFRAME);
            if(old || !GetLastError()){
                if(s->detached || !IsWindow(w)){
                    if(IsWindow(w)){
                        SetWindowLongPtrW(w,GWL_STYLE,GetWindowLongPtrW(w,GWL_STYLE)|WS_DLGFRAME);
                        result=DefSubclassProc(w,msg,wp,lp);
                    }
                    s->xaml_icon_update=FALSE;break;
                }
                result=DefSubclassProc(w,msg,wp,lp);DWORD error=GetLastError();
                if(IsWindow(w)){
                    LONG_PTR current=GetWindowLongPtrW(w,GWL_STYLE);
                    SetWindowLongPtrW(w,GWL_STYLE,current|WS_DLGFRAME);
                    if(!s->detached)SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|
                        SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
                }
                s->xaml_icon_update=FALSE;SetLastError(error);break;
            }
            s->xaml_icon_update=FALSE;
        }
        goto forward;
    }
    case WM_ERASEBKGND:
        result=DefSubclassProc(w,msg,wp,lp);
        /* Coalesce a burst of XAML background erases into one frame refresh.
         * Classic windows do not schedule this additional paint. */
        if(!s->detached && !s->xaml_repainting && !s->xaml_repaint_pending && !(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) &&
           win16_control_frame_xaml_bridge(w,FALSE)){
            (void)queue_repaint(w,s);
        }
        break;
    case WM_SHOWWINDOW:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    case WM_NCCALCSIZE:
    case WM_NCPAINT:
    case WM_SETCURSOR:
    case STYLE_NCUAHDRAWCAPTION:
    case STYLE_NCUAHDRAWFRAME:
    case STYLE_UAHDRAWMENU:
    case STYLE_UAHDRAWMENUITEM:
    case WM_NCACTIVATE:
    case WM_NCHITTEST:
    case WM_PRINT:
    case WM_SETTEXT:
    case WM_HSCROLL:
    case WM_VSCROLL:
    case WM_DPICHANGED:
    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
    case WM_THEMECHANGED: {
        if(win16_control_frame_message(w,s,msg,wp,lp,v->message,&result))break;
        goto forward;
    }
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
    case WM_MENUCHAR:
    case WM_INITMENUPOPUP:
    case WM_EXITMENULOOP:
        if(win16_control_menu_message(w,s,msg,wp,lp,v->paint_button,&result))break;
        goto forward;
    case WM_KEYDOWN:
    case WM_KILLFOCUS:
    case WM_NCMOUSEMOVE:
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCLBUTTONDOWN:
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:
    case WM_CANCELMODE:
    case WM_CAPTURECHANGED: {
        if(win16_control_frame_input(w,s,msg,wp,lp,v->input,&result))break;
        goto forward;
    }
    case WM_SYSCOMMAND:
        if(win16_control_frame_input(w,s,msg,wp,lp,v->input,&result))break;
        goto forward;
    case WM_MENUSELECT:
        if(*v->tracking_menu && (*v->tracking_menu)->window==w && lp)
            (*v->tracking_menu)->active=(HMENU)lp;
        goto forward;
    case WM_ENTERMENULOOP:
        win16_control_frame_state_enter_menu(s);
        goto forward;
    case WM_NCDESTROY:
        (void)v->terminal(w);
        result=DefSubclassProc(w,msg,wp,lp);break;
    default:
forward:
        result=DefSubclassProc(w,msg,wp,lp);break;
    }
done:
    if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
    return result;

}
