#include "frame_border.h"
#include <shellscalingapi.h>

void style_frame_solid_fill(HDC dc,RECT r,COLORREF c)
{
    SetDCBrushColor(dc,c);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
}

void style_frame_solid_outline(HDC dc,RECT r,int u,COLORREF c)
{
    RECT q=r;SetDCBrushColor(dc,c);HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    q.bottom=q.top+u;FillRect(dc,&q,brush);
    q=r;q.top=q.bottom-u;FillRect(dc,&q,brush);
    q=r;q.right=q.left+u;FillRect(dc,&q,brush);
    q=r;q.left=q.right-u;FillRect(dc,&q,brush);
}


SIZE style_frame_monitor_dpi(HWND window,UINT fallback)
{
    UINT x=fallback,y=fallback;
    DWORD error=GetLastError();
    DPI_AWARENESS_CONTEXT prior=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if(prior){
        HMONITOR monitor=MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST);
        if(FAILED(GetDpiForMonitor(monitor,MDT_EFFECTIVE_DPI,&x,&y)) || !x || !y)x=y=fallback;
        SetThreadDpiAwarenessContext(prior);
    }
    SetLastError(error);
    return (SIZE){(LONG)x,(LONG)y};
}

int style_frame_border_width(const style_frame_geometry *geometry,enum style_frame_border_kind kind,
    UINT coordinate_dpi,UINT monitor_dpi)
{
    const uint8_t widths[]={geometry->sizeable,geometry->fixed,geometry->dialog,geometry->line};
    const UINT source=geometry->scale==STYLE_FRAME_MONITOR_PIXELS?monitor_dpi:96;
    const int width=widths[kind];
    return width?max(1,MulDiv(width,(int)coordinate_dpi,(int)source)):0;
}

int style_frame_native_border_width(enum style_frame_border_kind kind,BOOL horizontal,UINT dpi)
{
    if(kind==STYLE_FRAME_LINE)return GetSystemMetricsForDpi(horizontal?SM_CXBORDER:SM_CYBORDER,dpi);
    if(kind==STYLE_FRAME_SIZEABLE)return GetSystemMetricsForDpi(horizontal?SM_CXSIZEFRAME:SM_CYSIZEFRAME,dpi)+
        GetSystemMetricsForDpi(SM_CXPADDEDBORDER,dpi);
    return GetSystemMetricsForDpi(horizontal?SM_CXFIXEDFRAME:SM_CYFIXEDFRAME,dpi);
}

int style_frame_window_border_width(HWND window,const style_frame_geometry *geometry,
    enum style_frame_border_kind kind,BOOL horizontal,UINT dpi)
{
    if(!geometry || !geometry->scale)return style_frame_native_border_width(kind,horizontal,dpi);
    UINT source=dpi;
    if(geometry->scale==STYLE_FRAME_MONITOR_PIXELS){
        SIZE monitor=style_frame_monitor_dpi(window,dpi);
        source=(UINT)(horizontal?monitor.cx:monitor.cy);
    }
    return style_frame_border_width(geometry,kind,dpi,source);
}

int style_frame_caption_height(int logical_height,UINT dpi,int minimum)
{
    return max(minimum,MulDiv(logical_height,(int)dpi,96));
}

static void stroke(HDC dc,RECT bounds,int width,COLORREF leading,COLORREF trailing)
{
    if(width<1 || IsRectEmpty(&bounds))return;
    int top_end=min(bounds.bottom,bounds.top+width), bottom_start=max(bounds.top,bounds.bottom-width);
    int left_end=min(bounds.right,bounds.left+width), right_start=max(bounds.left,bounds.right-width);
    RECT top={bounds.left,bounds.top,bounds.right,top_end};
    RECT left={bounds.left,top_end,left_end,bounds.bottom};
    RECT bottom={bounds.left,bottom_start,bounds.right,bounds.bottom};
    RECT right={right_start,bounds.top,bounds.right,bottom_start};
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    SetDCBrushColor(dc,leading);
    FillRect(dc,&top,brush);
    FillRect(dc,&left,brush);
    SetDCBrushColor(dc,trailing);
    FillRect(dc,&bottom,brush);
    FillRect(dc,&right,brush);
}

static void dividers(HDC dc,RECT bounds,int border,int unit,SIZE corner,COLORREF ink)
{
    if(border<=2*unit)return;
    int horizontal=corner.cx-unit,vertical=corner.cy-unit;
    int width=bounds.right-bounds.left,height=bounds.bottom-bounds.top;
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    SetDCBrushColor(dc,ink);
    for(unsigned i=0;i<2;++i){
        if(width>2*horizontal+unit){
            int x=i?bounds.right-horizontal-unit:bounds.left+horizontal;
            RECT mark={x,bounds.top+unit,x+unit,bounds.top+border-unit};
            FillRect(dc,&mark,brush);
            OffsetRect(&mark,0,height-border);
            FillRect(dc,&mark,brush);
        }
        if(height>2*vertical+unit){
            int y=i?bounds.bottom-vertical-unit:bounds.top+vertical;
            RECT mark={bounds.left+unit,y,bounds.left+border-unit,y+unit};
            FillRect(dc,&mark,brush);
            OffsetRect(&mark,width-border,0);
            FillRect(dc,&mark,brush);
        }
    }
}

void style_frame_border_paint(HDC dc,RECT bounds,int border,int unit,BOOL dialog,const SIZE *resize_corners,
    const style_border_presentation *presentation,const style_border_palette *palette)
{
    const COLORREF colors[]={palette->face,palette->edge,palette->light,palette->shadow,palette->title};
    COLORREF fill=colors[dialog?presentation->dialog_fill:STYLE_BORDER_FACE];
    if(presentation->fill==STYLE_BORDER_CLIPPED){
        SetDCBrushColor(dc,fill);
        FillRect(dc,&bounds,(HBRUSH)GetStockObject(DC_BRUSH));
    }else if(border>0 && bounds.right-bounds.left>2*border && bounds.bottom-bounds.top>2*border){
        stroke(dc,bounds,border,fill,fill);
    }
    style_border_stroke inner=presentation->inner;
    if(dialog && presentation->dialog_inner!=STYLE_BORDER_KEEP)
        inner.leading=inner.trailing=presentation->dialog_inner;
    const style_border_stroke strokes[]={presentation->outer,inner};
    const int offsets[]={0,unit,border-unit};
    for(unsigned i=0;i<ARRAYSIZE(strokes);++i){
        style_border_stroke paint=strokes[i];
        RECT layer=bounds;
        int offset=offsets[paint.anchor];
        InflateRect(&layer,-offset,-offset);
        stroke(dc,layer,unit,colors[paint.leading],colors[paint.trailing]);
    }
    if(!dialog && resize_corners && presentation->divider!=STYLE_BORDER_KEEP)
        dividers(dc,bounds,border,unit,*resize_corners,colors[presentation->divider]);
}
