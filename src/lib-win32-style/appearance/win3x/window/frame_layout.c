#include "frame_layout.h"
#include "lib-win32-style/application/menu/menu_input.h"
#include "lib-win32-style/application/window/dwm.h"
#include "lib-win32-style/application/window/frame.h"
#include <dwmapi.h>
#include "../menu/menu_bar.h"
#include <windowsx.h>

BOOL win16_control_frame_hit_test(HWND w,const win16_style_layout *l,LPARAM lp,LRESULT *result)
{
    RECT r;POINT point;
    POINT screen={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
    if(style_frame_scrollbar_hit(w,screen,result))return TRUE;
    if(!GetWindowRect(w,&r))return FALSE;
    point.x=screen.x-r.left;point.y=screen.y-r.top;
    *result=style_menu_neutral_hit(PtInRect(&l->menu,point)?HTMENU:win16_style_hit(l,point));
    return TRUE;
}

BOOL win16_control_measure_window(HWND w,const style_instance *instance,const style_font_pair *fonts,
    const RECT *outer,const classic_button_group *buttons,const classic_caption_metrics *metrics,
    const style_frame_geometry *frame,unsigned icon_mode,const classic_menu_metrics *menu,win16_style_layout *l)
{
    RECT visible=*outer;BOOL zoom=IsZoomed(w);int dx,dy;
    if(zoom)(void)style_frame_work_area(w,outer,&visible);
    DWORD style=(DWORD)GetWindowLongPtr(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtr(w,GWL_EXSTYLE);
    int width=visible.right-visible.left,height=visible.bottom-visible.top;
    UINT monitor=frame && frame->scale==STYLE_FRAME_MONITOR_PIXELS?
        (UINT)style_frame_monitor_dpi(w,fonts->dpi).cy:fonts->dpi;
    BOOL measured=win16_style_measure_with_buttons(width,height,fonts->dpi,style,ex,zoom,
        buttons,metrics,frame,monitor,l);
    if(!measured)return FALSE;
    win16_style_measure_caption_icon(l,fonts->dpi,style,ex,icon_mode);
    if(menu)l->menu_metrics=*menu;
    /* Native maximized resize borders can lie outside the work area. Keep
     * Windows' geometry, but place every custom non-client element and the
     * actual client inside its visible part using one layout calculation. */
    dx=visible.left-outer->left;dy=visible.top-outer->top;
    OffsetRect(&l->outer,dx,dy);OffsetRect(&l->client,dx,dy);
    OffsetRect(&l->caption,dx,dy);OffsetRect(&l->title,dx,dy);OffsetRect(&l->icon,dx,dy);
    for(unsigned role=CLASSIC_BUTTON_SYSTEM;role<=CLASSIC_BUTTON_CLOSE;++role)
        OffsetRect(&l->buttons[role],dx,dy);
    if(!(GetWindowLongPtr(w,GWL_STYLE)&WS_CHILD) && GetMenu(w)){
        style_menu_bar_snapshot bar;POINT origin={l->client.left,l->client.top};
        if(!win16_style_bar_read_for_window(w,GetMenu(w),fonts->menu?fonts->menu:(HFONT)GetStockObject(SYSTEM_FONT),
            fonts->dpi,l,origin,l->client.right-l->client.left,&bar))return FALSE;
        if(instance->detached){style_menu_bar_free(&bar);return FALSE;}
        if(bar.height){l->menu=l->client;l->menu.bottom=min(l->client.bottom,l->client.top+bar.height+l->unit);
            l->client.top=l->menu.bottom;}
        style_menu_bar_free(&bar);
    }
    {DWORD flags=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        BOOL left_bar=(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_LEFTSCROLLBAR)!=0;
        int shared=frame && l->sizeable && !l->dialog_frame && !left_bar?
            frame->scroll_shared_units*l->unit:0;
        if(flags&WS_VSCROLL){int cx=GetSystemMetricsForDpi(SM_CXVSCROLL,fonts->dpi)-shared;
            if(l->client.right-l->client.left>=cx){
                if(left_bar)l->client.left+=cx;
                else l->client.right-=cx;}}
        if(flags&WS_HSCROLL){int cy=GetSystemMetricsForDpi(SM_CYHSCROLL,fonts->dpi)-shared;
            if(l->client.bottom-l->client.top>cy)l->client.bottom-=cy;}
    }
    return TRUE;
}
