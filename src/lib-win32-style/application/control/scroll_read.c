#include "scroll_read.h"

RECT style_scroll_caret_rectangle(const style_scroll_layout *layout,const style_scroll_track *track,int inset)
{
    RECT rectangle=track->active && track->part==STYLE_SCROLL_THUMB?
        style_scroll_thumb_at_pixel(layout,track->pixel):layout->thumb;
    InflateRect(&rectangle,-inset,-inset);
    return rectangle;
}

BOOL style_scroll_read_control_layout(HWND window,style_scroll_info_fn get_info,
    style_scroll_metrics metrics,style_scroll_defaults defaults,style_scroll_layout *out)
{
    RECT bar;SCROLLINFO info;UINT dpi;
    if(!style_scroll_control_read(window,get_info,&bar,&info,&dpi))return FALSE;
    BOOL vertical=(GetWindowLongPtrW(window,GWL_STYLE)&SBS_VERT)!=0;
    return style_scroll_measure_metrics(&bar,vertical,dpi,&info,metrics,defaults,out);
}

BOOL style_scroll_read_embedded_layout(HWND window,LONG object,style_scroll_info_fn get_info,
    style_scroll_metrics metrics,style_scroll_defaults defaults,style_scroll_layout *out,POINT *origin)
{
    RECT bar;SCROLLINFO info;UINT dpi;
    if(!style_scroll_embedded_read(window,object,get_info,&bar,origin,&info,&dpi))return FALSE;
    return style_scroll_measure_metrics(&bar,object==OBJID_VSCROLL,dpi,&info,metrics,defaults,out);
}
