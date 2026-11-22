#include "native_scroll.h"
#include "lib-win32-style/application/control/scroll_surface.h"

static BOOL read_info(HWND window,int bar,SCROLLINFO *info)
{
    return GetScrollInfo(window,bar,info);
}

static BOOL vertical_axis(HWND window,LONG object)
{
    return object==OBJID_VSCROLL ||
        (object==OBJID_CLIENT && (GetWindowLongPtrW(window,GWL_STYLE)&SBS_VERT));
}

static BOOL measure(HWND window,const style_native_scroll *scroll,LONG object,const RECT *bar,style_scroll_layout *out)
{
    BOOL vertical=vertical_axis(window,object);
    UINT dpi=GetDpiForWindow(window);
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS};
    if(!dpi)dpi=96;
    int axis=object==OBJID_CLIENT?SB_CTL:vertical?SB_VERT:SB_HORZ;
    return read_info(window,axis,&info) &&
        style_scroll_measure_metrics(bar,vertical,dpi,&info,scroll->metrics,
            (style_scroll_defaults){STYLE_SCROLL_EXTENT_SYSTEM,STYLE_SCROLL_THUMB_PAGE},out);
}

static BOOL layout(HWND window,void *context,LONG object,style_scroll_layout *out,POINT *origin)
{
    style_scroll_surface surface;
    if(!style_scroll_surface_read(window,object,&surface))return FALSE;
    if(origin)*origin=(POINT){surface.info.rcScrollBar.left,surface.info.rcScrollBar.top};
    return measure(window,context,object,&surface.bar,out);
}

static void arrow(HDC dc,RECT r,UINT state,const style_native_scroll *scroll)
{
    if(IsRectEmpty(&r))return;
    const style_relief_palette *p=&scroll->arrow_palette;
    if(!style_paint_scroll_arrow(dc,&r,state,p,&scroll->arrow_art)){
        UINT edge=(state&DFCS_PUSHED)?EDGE_SUNKEN:EDGE_RAISED;
        style_paint_relief(dc,&r,edge,BF_RECT|BF_ADJUST,p);
        style_paint_color(dc,&r,p->face);
    }
}

static HBRUSH track_brush(HWND window,HDC dc)
{
    HWND parent=GetParent(window);
    if(!parent)return NULL;
    HBRUSH standard=(HBRUSH)DefWindowProcW(window,WM_CTLCOLORSCROLLBAR,(WPARAM)dc,(LPARAM)window);
    HBRUSH brush=(HBRUSH)SendMessageW(parent,WM_CTLCOLORSCROLLBAR,(WPARAM)dc,(LPARAM)window);
    return brush!=standard && GetObjectType(brush)==OBJ_BRUSH?brush:NULL;
}

static void paint_track(HDC dc,RECT r,BOOL vertical,HBRUSH brush,const style_relief_palette *p)
{
    if(IsRectEmpty(&r))return;
    if(!brush){style_paint_dither(dc,&r,p);return;}
    FillRect(dc,&r,brush);
    int width=vertical?r.right-r.left:r.bottom-r.top;
    for(int layer=0;layer<2 && width>2*layer;++layer){
        RECT first=r,last=r;
        if(vertical){
            first.left+=layer;
            first.right=first.left+1;
            last.right-=layer;
            last.left=last.right-1;
        }else{
            first.top+=layer;
            first.bottom=first.top+1;
            last.bottom-=layer;
            last.top=last.bottom-1;
        }
        COLORREF color=layer?p->face:p->shadow;
        style_paint_color(dc,&first,color);
        style_paint_color(dc,&last,color);
    }
}

static style_relief_palette surface_palette(const style_scroll_surface_colors *surface)
{
    style_relief_palette palette=style_relief_native_palette();
    if(surface){
        palette.face=surface->face;
        palette.light=surface->light;
        palette.highlight=surface->highlight;
        palette.shadow=surface->shadow;
        palette.dark=surface->dark;
    }
    return palette;
}

static void draw(HWND window,void *context,HDC dc,LONG object,BOOL vertical)
{
    style_native_scroll *scroll=context;
    style_scroll_surface surface;
    style_scroll_layout l;
    if(!dc || scroll->instance->detached || !style_scroll_surface_read(window,object,&surface) ||
        !measure(window,scroll,object,&surface.target,&l) || scroll->instance->detached)return;
    int saved=SaveDC(dc);
    if(!saved)return;
    const style_native_scroll_colors *colors=&scroll->colors;
    style_relief_palette track_colors=surface_palette(colors->tracks_and_thumbs?&colors->track[!!vertical]:NULL);
    HBRUSH brush=object==OBJID_CLIENT?track_brush(window,dc):NULL;
    if(scroll->instance->detached){RestoreDC(dc,saved);return;}
    IntersectClipRect(dc,l.bar.left,l.bar.top,l.bar.right,l.bar.bottom);
    style_scroll_track *track=&scroll->controller.track;
    BOOL tracking=track->active && (object==OBJID_CLIENT || scroll->controller.object==object);
    style_scroll_part part=tracking && track->inside?track->part:STYLE_SCROLL_NONE;
    UINT disabled=style_scroll_disabled(window,object);
    UINT first=vertical?DFCS_SCROLLUP:DFCS_SCROLLLEFT,last=vertical?DFCS_SCROLLDOWN:DFCS_SCROLLRIGHT;
    if(disabled&ESB_DISABLE_UP)first|=DFCS_INACTIVE;
    if(disabled&ESB_DISABLE_DOWN)last|=DFCS_INACTIVE;
    if(part==STYLE_SCROLL_LINE_BEFORE)first|=DFCS_PUSHED|DFCS_FLAT;
    if(part==STYLE_SCROLL_LINE_AFTER)last|=DFCS_PUSHED|DFCS_FLAT;
    RECT groove=l.bar;
    if(vertical){
        groove.top=l.first.bottom;
        groove.bottom=l.last.top;
    }else{
        groove.left=l.first.right;
        groove.right=l.last.left;
    }
    paint_track(dc,groove,vertical,brush,&track_colors);
    if(l.thumb_fits && disabled!=ESB_DISABLE_BOTH){
        style_relief_palette thumb_colors=surface_palette(colors->tracks_and_thumbs?&colors->thumb[!!vertical]:NULL);
        RECT thumb=l.thumb;
        if(tracking && track->part==STYLE_SCROLL_THUMB)
            thumb=style_scroll_thumb_at_pixel(&l,(vertical?surface.origin.y:surface.origin.x)+track->pixel);
        RECT page=groove;
        if(part==STYLE_SCROLL_PAGE_BEFORE){
            if(vertical)page.bottom=thumb.top;else page.right=thumb.left;
            InvertRect(dc,&page);
        }else if(part==STYLE_SCROLL_PAGE_AFTER){
            if(vertical)page.top=thumb.bottom;else page.left=thumb.right;
            InvertRect(dc,&page);
        }
        style_paint_relief(dc,&thumb,EDGE_RAISED,BF_RECT|BF_ADJUST,&thumb_colors);
        style_paint_color(dc,&thumb,thumb_colors.face);
    }
    arrow(dc,l.first,first,scroll);
    arrow(dc,l.last,last,scroll);
    RestoreDC(dc,saved);
}

static void sizebox(HDC dc,RECT r,BOOL grip,const style_native_scroll *scroll)
{
    style_relief_palette palette=surface_palette(scroll->colors.own_sizebox?&scroll->colors.sizebox:NULL);
    style_paint_color(dc,&r,palette.face);
    if(!grip)return;
    int size=min(r.right-r.left,r.bottom-r.top);
    if(size<=0)return;
    int saved=SaveDC(dc);
    if(!saved)return;
    HFONT font=CreateFontW(size,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,SYMBOL_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,0,L"Marlett");
    if(font){
        HGDIOBJ old=SelectObject(dc,font);
        WCHAR family[32];
        if(old && old!=HGDI_ERROR && GetTextFaceW(dc,ARRAYSIZE(family),family) && !lstrcmpiW(family,L"Marlett")){
            SetBkMode(dc,TRANSPARENT);
            SetTextAlign(dc,TA_LEFT|TA_TOP);
            SetTextColor(dc,palette.highlight);
            TextOutW(dc,r.right-size,r.bottom-size,L"o",1);
            SetTextColor(dc,palette.shadow);
            TextOutW(dc,r.right-size,r.bottom-size,L"p",1);
        }
    }
    RestoreDC(dc,saved);
    if(font)DeleteObject(font);
}

void style_native_scroll_sizebox(HWND window,void *context,HDC dc,BOOL buffered)
{
    (void)buffered;
    RECT bounds;
    if(dc && GetClientRect(window,&bounds))
        sizebox(dc,bounds,(GetWindowLongPtrW(window,GWL_STYLE)&SBS_SIZEGRIP)!=0,context);
}

void style_native_scroll_paint_nc(HWND window,style_native_scroll *scroll,HDC borrowed)
{
    if(scroll->instance->detached || (GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL) ||
        !(GetWindowLongPtrW(window,GWL_STYLE)&(WS_HSCROLL|WS_VSCROLL)))return;
    HDC dc=borrowed?borrowed:GetWindowDC(window);
    if(!dc)return;
    draw(window,scroll,dc,OBJID_HSCROLL,FALSE);
    draw(window,scroll,dc,OBJID_VSCROLL,TRUE);
    style_scroll_surface horizontal,vertical;
    if(style_scroll_surface_read(window,OBJID_HSCROLL,&horizontal) &&
       style_scroll_surface_read(window,OBJID_VSCROLL,&vertical)){
        RECT corner={vertical.target.left,horizontal.target.top,vertical.target.right,horizontal.target.bottom};
        LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
        if(!IsRectEmpty(&corner))sizebox(dc,corner,(style&(WS_CHILD|WS_THICKFRAME))==WS_THICKFRAME && !IsZoomed(window),scroll);
    }
    if(!borrowed)ReleaseDC(window,dc);
}

static void nonclient(HWND window,void *context,HDC dc)
{
    style_native_scroll_paint_nc(window,context,dc);
}

static void caret(HWND window,void *context)
{
    style_native_scroll *scroll=context;
    style_scroll_controller *c=&scroll->controller;
    if(c->resources.edit || c->resources.list || c->resources.frame)return;
    style_scroll_layout l;
    RECT r;
    BOOL show=!scroll->instance->detached && GetFocus()==window && IsWindowEnabled(window) &&
        style_scroll_disabled(window,OBJID_CLIENT)!=ESB_DISABLE_BOTH &&
        layout(window,context,OBJID_CLIENT,&l,NULL) && l.thumb_fits;
    if(show){
        r=l.thumb;
        InflateRect(&r,-2,-2);
    }
    style_scroll_caret_sync(window,&c->resources,show?&r:NULL,c->track.active);
}

void style_native_scroll_init(style_native_scroll *scroll,style_instance *instance)
{
    scroll->instance=instance;
    scroll->arrow_palette=style_relief_native_palette();
    scroll->arrow_art=(style_scroll_arrow_art){0};
    scroll->colors=(style_native_scroll_colors){0};
    scroll->policy=(style_scroll_policy){.hit=style_scroll_hit};
    style_scroll_repeat_configure(&scroll->policy,0,0);
}

void style_native_scroll_configure(style_native_scroll *scroll,style_relief_palette arrows,
    const style_scroll_arrow_art *art,const style_native_scroll_colors *colors,style_scroll_metrics metrics)
{
    scroll->arrow_palette=arrows;
    scroll->arrow_art=*art;
    scroll->colors=*colors;
    scroll->metrics=metrics;
    scroll->policy.drag=scroll->metrics.drag;
    style_scroll_repeat_configure(&scroll->policy,scroll->metrics.initial_repeat_ms,scroll->metrics.repeat_ms);
}

style_scroll_ops style_native_scroll_operations(style_native_scroll *scroll)
{
    return (style_scroll_ops){scroll,&scroll->policy,read_info,layout,draw,nonclient,caret};
}

void style_native_scroll_retire(HWND window,style_native_scroll *scroll,BOOL terminal)
{
    style_scroll_controller *c=&scroll->controller;
    if(terminal){
        style_scroll_stop_timer(window,&c->resources.timer);
        if(c->track.active)style_scroll_release_capture(window,&c->resources.previous_capture,
            c->resources.edit || c->resources.list || c->resources.frame);
    }else{
        const style_scroll_ops ops=style_native_scroll_operations(scroll);
        style_scroll_finish(window,scroll->instance,c,&ops,TRUE);
    }
    style_scroll_caret_destroy(window,&c->resources);
}
