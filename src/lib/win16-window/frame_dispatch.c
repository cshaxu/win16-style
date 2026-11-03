#include "frame_dispatch.h"
#include <commctrl.h>
#include <windowsx.h>
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
    /* Our menu bar is custom nonclient chrome. USER hot-tracks HTMENU before
     * NC mouse dispatch (classic/WOW has no UAH draw callback to intercept).
     * Expose neutral nonclient hit-test ownership and route clicks by our
     * measured menu rectangle; never alter the application's HMENU. */
    if((msg==WM_NCLBUTTONDOWN || msg==WM_NCLBUTTONDBLCLK) && wp==HTBORDER && v->input->layout(w,s,&l)){
        GetWindowRect(w,&r);point.x=GET_X_LPARAM(lp)-r.left;point.y=GET_Y_LPARAM(lp)-r.top;
        if(PtInRect(&l.menu,point))wp=HTMENU;
    }
    switch(msg){
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
    if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
    return result;

}
