#include "scroll_native.h"
#include "lib-win32-style/application/control/scroll_state.h"
/* Pinned USER18 initial/repeat delay and 4*CXBORDER/CYBORDER drag tolerance. */
const style_scroll_policy win16_scroll_policy={200,50,4,1,win16_scroll_hit};
RECT win16_scroll_caret_rectangle(HWND window,const style_scroll_layout *layout,const style_scroll_track *track)
{
    RECT rectangle=layout->thumb;
    if(track->active && track->part==STYLE_SCROLL_THUMB){
        if(layout->vertical)OffsetRect(&rectangle,0,track->pixel-rectangle.top);
        else OffsetRect(&rectangle,track->pixel-rectangle.left,0);}
    int inset=max(1,MulDiv(2,(int)GetDpiForWindow(window),96));
    InflateRect(&rectangle,-inset,-inset);return rectangle;
}
BOOL win16_scroll_control_layout(HWND w,win16_scroll_info_fn get_info,style_scroll_layout *layout)
{
    RECT r;SCROLLINFO info;UINT dpi;
    if(!style_scroll_control_read(w,get_info,&r,&info,&dpi))return FALSE;
    return win16_scroll_measure_native(&r,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0,dpi,&info,layout);
}
BOOL win16_scroll_embedded_layout(HWND w,LONG object,win16_scroll_info_fn get_info,
    style_scroll_layout *layout,POINT *screen_origin)
{
    BOOL vertical=object==OBJID_VSCROLL;
    SCROLLINFO info;RECT r;UINT dpi;
    if(!style_scroll_embedded_read(w,object,get_info,&r,screen_origin,&info,&dpi))return FALSE;
    return win16_scroll_measure_native(&r,vertical,dpi,&info,layout);
}
BOOL win16_scroll_measure_native(const RECT *bar,BOOL vertical,UINT dpi,
    const SCROLLINFO *info,style_scroll_layout *layout)
{
    if(!bar || !info)return FALSE;
    win16_scroll_metrics metrics={MulDiv(17,(int)dpi,96),MulDiv(17,(int)dpi,96),max(1,MulDiv(1,(int)dpi,96))};
    metrics.arrow=metrics.thumb=vertical?bar->right-bar->left:bar->bottom-bar->top;
    LONGLONG maximum=(LONGLONG)info->nMax-(info->nPage?info->nPage-1:0);
    if(maximum<info->nMin)maximum=info->nMin;
    return win16_scroll_measure(bar,vertical,&metrics,info->nMin,(LONG)maximum,info->nPos,layout);
}

void win16_control_managed_frame_scrollbars_palette_owned(HWND w,const win16_style_palette *palette,const void *owner,const style_scroll_managed_ops *ops)
{
    style_scroll_binding binding=ops->lookup(w);
    if(binding.instance && binding.scroll->resources.frame && binding.instance->owner==owner)
        ((control_style *)binding.instance->context)->palette=*palette;
}
void win16_control_managed_frame_scrollbars_palette(HWND w,const win16_style_palette *palette,const style_scroll_managed_ops *ops)
{
    style_scroll_binding binding=ops->lookup(w);
    if(binding.instance && binding.scroll->resources.frame)((control_style *)binding.instance->context)->palette=*palette;
}
