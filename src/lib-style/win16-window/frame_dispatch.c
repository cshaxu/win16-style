#include "lib-style/common/frame.h"
#include "frame_dispatch.h"
#include "frame_layout.h"
#include "lib-style/common/menu_input.h"
#include <commctrl.h>
#include <windowsx.h>
static void repaint_retain(void *context){++((style_window *)context)->calls;}
static void repaint_release(void *context)
{style_window *s=context;if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);}
static void repaint_activate(HWND owner,void *context)
{
    style_window *s=context;s->pending_frame=FALSE;
    RECT client;BOOL normal=!IsIconic(owner)&&!IsZoomed(owner);
    style_client_screen(owner,&client);s->xaml_activate(owner,s,&client,normal);
}
static const style_uwp_repaint_services repaint_services={repaint_retain,repaint_release,repaint_activate};
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
        if(s->xaml.waiting){if(!wp)style_uwp_repaint_cancel(&s->xaml);goto forward;}
        if(wp && style_frame_xaml_bridge(w,FALSE)){
            s->xaml.waiting=TRUE;s->xaml.deadline=GetTickCount64()+2000;s->xaml_activate=v->message->activate;
            if(style_uwp_repaint_queue(w,&s->xaml,&s->detached,s,&repaint_services))goto forward;
            s->xaml.waiting=FALSE;
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
        if(style_frame_xaml_icon(w,msg,wp,lp,&s->xaml_icon_update,&s->detached,&result))break;
        goto forward;
    }
    case WM_ERASEBKGND:
        result=DefSubclassProc(w,msg,wp,lp);
        /* Coalesce a burst of XAML background erases into one frame refresh.
         * Classic windows do not schedule this additional paint. */
        if(!s->detached && !s->xaml.repainting && !s->xaml.pending && !(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) &&
           style_frame_xaml_bridge(w,FALSE)){
            (void)style_uwp_repaint_queue(w,&s->xaml,&s->detached,s,&repaint_services);
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
