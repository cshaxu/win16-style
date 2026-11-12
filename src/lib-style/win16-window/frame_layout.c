#include "frame_layout.h"
#include "lib-style/common/menu_input.h"
#include "lib-style/common/dwm.h"
#include "lib-style/common/frame.h"
#include <dwmapi.h>
#include "menu_bar.h"
#include <windowsx.h>

BOOL win16_control_frame_set_cursor(HWND w,WPARAM target,LPARAM hit)
{
    if(!IsIconic(w) && (HWND)target==w && (short)LOWORD(hit)>HTCLIENT){
        LPCWSTR cursor=style_frame_resize_cursor((short)LOWORD(hit));
        SetCursor(LoadCursorW(NULL,cursor));return TRUE;
    }
    return FALSE;
}
void win16_control_frame_activate(HWND w,const BOOL *detached,const RECT *client,BOOL normal,win16_control_set_window_attribute set_attribute)
{
    DWORD policy=DWMNCRP_DISABLED;
    if(!(GetWindowLongPtr(w,GWL_STYLE)&WS_CHILD))
        style_dwm_policy(w,set_attribute,policy);
    if(!*detached)style_frame_changed(w);
    if(!*detached && normal)style_restore_client(w,client);
}
void win16_control_frame_restore(HWND w,const BOOL *pending,const BOOL *policy_saved,const DWORD *old_policy,const RECT *client,BOOL normal,win16_control_set_window_attribute set_attribute)
{
    if(IsWindow(w) && !*pending && *policy_saved)style_dwm_policy(w,set_attribute,*old_policy);
    if(IsWindow(w) && !*pending)style_frame_changed_restore(w,client,normal);
}
BOOL win16_control_frame_hit_test(HWND w,const win16_style_layout *l,LPARAM lp,LRESULT *result)
{
    RECT r;POINT point;SCROLLBARINFO scroll={.cbSize=sizeof(scroll)};
    POINT screen={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
    DWORD flags=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    if((flags&WS_HSCROLL) && GetScrollBarInfo(w,OBJID_HSCROLL,&scroll) && PtInRect(&scroll.rcScrollBar,screen)){
        *result=HTHSCROLL;return TRUE;}
    if((flags&WS_VSCROLL) && GetScrollBarInfo(w,OBJID_VSCROLL,&scroll) && PtInRect(&scroll.rcScrollBar,screen)){
        *result=HTVSCROLL;return TRUE;}
    if(!GetWindowRect(w,&r))return FALSE;
    point.x=screen.x-r.left;point.y=screen.y-r.top;
    *result=style_menu_neutral_hit(PtInRect(&l->menu,point)?HTMENU:win16_style_hit(l,point));
    return TRUE;
}

BOOL win16_control_measure_window(HWND w,const UINT *dpi,HFONT *menu_font,const BOOL *detached,const RECT *outer,win16_style_layout *l)
{
    RECT visible=*outer;BOOL zoom=IsZoomed(w);int dx,dy;
    if(zoom)(void)style_frame_work_area(w,outer,&visible);
    if(!win16_style_measure(visible.right-visible.left,visible.bottom-visible.top,
        *dpi,(DWORD)GetWindowLongPtr(w,GWL_STYLE),
        (DWORD)GetWindowLongPtr(w,GWL_EXSTYLE),zoom,l))return FALSE;
    /* Native maximized resize borders can lie outside the work area. Keep
     * Windows' geometry, but place every custom non-client element and the
     * actual client inside its visible part using one layout calculation. */
    dx=visible.left-outer->left;dy=visible.top-outer->top;
    OffsetRect(&l->outer,dx,dy);OffsetRect(&l->client,dx,dy);
    OffsetRect(&l->caption,dx,dy);OffsetRect(&l->title,dx,dy);
    OffsetRect(&l->system,dx,dy);OffsetRect(&l->minimize,dx,dy);OffsetRect(&l->maximize,dx,dy);
    if(!(GetWindowLongPtr(w,GWL_STYLE)&WS_CHILD) && GetMenu(w)){
        win16_style_bar bar;POINT origin={l->client.left,l->client.top};
        if(!win16_style_bar_read_for_window(w,GetMenu(w),*menu_font?*menu_font:(HFONT)GetStockObject(SYSTEM_FONT),
            *dpi,origin,l->client.right-l->client.left,&bar))return FALSE;
        if(*detached){win16_style_bar_free(&bar);return FALSE;}
        if(bar.height){l->menu=l->client;l->menu.bottom=min(l->client.bottom,l->client.top+bar.height+l->unit);
            l->client.top=l->menu.bottom;}
        win16_style_bar_free(&bar);
    }
    {DWORD flags=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        /* USER18:0F5F uses the inner sizing band N=F-B for the bars' outer
         * edges: the bar shares the frame's inner border. Native bar geometry
         * starts at rcClient, so reserve thickness minus that shared unit.
         * Fixed/dialog and host-only left-bar profiles remain separate. */
        int shared=l->sizeable && !l->dialog_frame &&
            !(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_LEFTSCROLLBAR)?l->unit:0;
        if(flags&WS_VSCROLL){int cx=GetSystemMetricsForDpi(SM_CXVSCROLL,*dpi)-shared;
            if(l->client.right-l->client.left>=cx){
                if(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_LEFTSCROLLBAR)l->client.left+=cx;
                else l->client.right-=cx;}}
        if(flags&WS_HSCROLL){int cy=GetSystemMetricsForDpi(SM_CYHSCROLL,*dpi)-shared;
            if(l->client.bottom-l->client.top>cy)l->client.bottom-=cy;}
    }
    return TRUE;
}
