#include "frame_layout.h"
#include "menu_bar.h"

BOOL win16_control_measure_window(HWND w,const UINT *dpi,HFONT *menu_font,const BOOL *detached,const RECT *outer,win16_style_layout *l)
{
    RECT visible=*outer;BOOL zoom=IsZoomed(w);int dx,dy;
    if(zoom){
        MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT intersection;
        if(GetMonitorInfoW(MonitorFromWindow(w,MONITOR_DEFAULTTONEAREST),&monitor) &&
            IntersectRect(&intersection,outer,&monitor.rcWork))visible=intersection;
    }
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
