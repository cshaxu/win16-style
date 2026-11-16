#include "scroll_surface.h"
BOOL style_scroll_surface_read(HWND window,LONG object,style_scroll_surface *surface)
{
    *surface=(style_scroll_surface){.info.cbSize=sizeof(surface->info)};
    if(!GetScrollBarInfo(window,object,&surface->info) || (surface->info.rgstate[0]&STATE_SYSTEM_INVISIBLE))return FALSE;
    if(object==OBJID_CLIENT){
        if(!GetClientRect(window,&surface->bar))return FALSE;
        surface->target=surface->bar;
    }else{
        RECT outer;if(!GetWindowRect(window,&outer))return FALSE;
        surface->origin=(POINT){surface->info.rcScrollBar.left-outer.left,surface->info.rcScrollBar.top-outer.top};
        SetRect(&surface->bar,0,0,surface->info.rcScrollBar.right-surface->info.rcScrollBar.left,
            surface->info.rcScrollBar.bottom-surface->info.rcScrollBar.top);
        surface->target=surface->info.rcScrollBar;OffsetRect(&surface->target,-outer.left,-outer.top);
    }
    return !IsRectEmpty(&surface->target);
}
