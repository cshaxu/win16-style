#include "layout.h"
#include "lib-win32-style/application/window/caption_paint.h"
#include <string.h>

BOOL win16_style_measure(int width,int height,UINT dpi,DWORD style,DWORD ex_style,
    BOOL maximized,win16_style_layout *l)
{
    return win16_style_measure_with_buttons(width,height,dpi,style,ex_style,maximized,NULL,NULL,NULL,dpi,l);
}

BOOL win16_style_measure_with_buttons(int width,int height,UINT dpi,DWORD style,DWORD ex_style,
    BOOL maximized,const classic_button_group *group,const classic_caption_metrics *metrics,
    const style_frame_geometry *frame,UINT monitor_dpi,win16_style_layout *l)
{
    const BOOL caption=(style&WS_CAPTION)==WS_CAPTION;
    if(!l || width<1 || height<1 || dpi<48 || dpi>768)return FALSE;
    const BOOL themed_frame=frame && frame->scale;
    memset(l,0,sizeof(*l));
    l->button_size=(SIZE){
        metrics && metrics->button.cx?MulDiv(metrics->button.cx,(int)dpi,96):GetSystemMetricsForDpi(SM_CXSIZE,dpi),
        metrics && metrics->button.cy?MulDiv(metrics->button.cy,(int)dpi,96):GetSystemMetricsForDpi(SM_CYSIZE,dpi)};
    int button=l->button_size.cx;
    /* Round the one-pixel stroke like the scaled OEM separator, not down.
     * floor(dpi/96) left a one-column gap at 144 DPI. */
    l->unit=max(1,MulDiv(1,(int)dpi,96));
    l->sizeable=(style&WS_THICKFRAME)!=0 && !maximized;
    l->dialog_frame=(ex_style&WS_EX_DLGMODALFRAME)!=0;
    if(caption || l->dialog_frame || l->sizeable || (style&(WS_DLGFRAME|WS_BORDER))){
        enum style_frame_border_kind kind=l->dialog_frame?STYLE_FRAME_DIALOG:l->sizeable?STYLE_FRAME_SIZEABLE:
            caption || (style&WS_DLGFRAME)?STYLE_FRAME_FIXED:STYLE_FRAME_LINE;
        l->border=themed_frame?style_frame_border_width(frame,kind,dpi,monitor_dpi):
            style_frame_native_border_width(kind,FALSE,dpi);
    }
    int caption_height=metrics && metrics->height?MulDiv(metrics->height,(int)dpi,96):
        GetSystemMetricsForDpi(SM_CYCAPTION,dpi);
    if(caption && themed_frame)caption_height=max(caption_height,frame->caption_floor);
    int top=l->border;
    int separator=themed_frame?l->unit*frame->separator_units:0;
    l->corner.cx=l->border+button;
    l->corner.cy=top+caption_height+separator;
    SetRect(&l->outer,0,0,width,height);
    if(!caption){
        SetRect(&l->client,l->border,l->border,max(l->border,width-l->border),
            max(l->border,height-l->border));
        return TRUE;
    }
    SetRect(&l->caption,l->border,top,max(l->border,width-l->border),top+caption_height);
    SetRect(&l->client,l->border,l->corner.cy,
        max(l->border,width-l->border),max(l->corner.cy,height-l->border));
    l->title=l->caption;
    if(!group || !group->count)return TRUE;
    const unsigned mask=((style&WS_SYSMENU)?((1u<<CLASSIC_BUTTON_SYSTEM)|(1u<<CLASSIC_BUTTON_CLOSE) |
        ((ex_style&WS_EX_CONTEXTHELP)?1u<<CLASSIC_BUTTON_HELP:0)):0) |
        ((style&WS_MINIMIZEBOX)?1u<<CLASSIC_BUTTON_MINIMIZE:0) |
        ((style&WS_MAXIMIZEBOX)?1u<<CLASSIC_BUTTON_MAXIMIZE:0);
    int button_height=group->layout&CLASSIC_BUTTON_LAYOUT_EXPLICIT?min(caption_height,l->button_size.cy):caption_height;
    if(!style_caption_slots(group,&l->caption,dpi,(SIZE){button,button_height},mask,TRUE,l->buttons))
        return FALSE;
    for(unsigned i=0;i<group->count;++i){
        classic_button_slot slot=group->slots[i];
        if(IsRectEmpty(&l->buttons[slot.role]))continue;
        if(slot.side==CLASSIC_BUTTON_LEFT)l->title.left=max(l->title.left,l->buttons[slot.role].right);
        else l->title.right=min(l->title.right,l->buttons[slot.role].left);
    }
    l->title.right=max(l->title.left,l->title.right);
    return TRUE;
}

void win16_style_measure_caption_icon(win16_style_layout *l,UINT dpi,DWORD style,DWORD extended,unsigned mode)
{
    RECT band=l->caption;
    band.left=l->title.left;
    if(style_caption_icon_visible(mode,!IsRectEmpty(&l->buttons[CLASSIC_BUTTON_SYSTEM]),style,extended) &&
       style_caption_icon_bounds(band,l->title.right,dpi,&l->icon))
        l->title.left=max(l->title.left,l->icon.right);
}

LRESULT win16_style_hit(const win16_style_layout *l,POINT p)
{
    BOOL left,right,top,bottom;
    if(!l || !PtInRect(&l->outer,p))return HTNOWHERE;
    int button=style_caption_hit(l->buttons+CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_SYSTEM,~0u,p);
    if(button)return button;
    if(PtInRect(&l->icon,p))return HTSYSMENU;
    if(PtInRect(&l->caption,p))return HTCAPTION;
    if(PtInRect(&l->client,p))return HTCLIENT;
    if(!l->sizeable)return HTBORDER;
    left=p.x<l->corner.cx;right=p.x>=l->outer.right-l->corner.cx+l->unit;
    top=p.y<l->border;bottom=p.y>=l->outer.bottom-l->border;
    if(top)return left?HTTOPLEFT:right?HTTOPRIGHT:HTTOP;
    if(bottom)return left?HTBOTTOMLEFT:right?HTBOTTOMRIGHT:HTBOTTOM;
    /* USER 1:67BB..6859 extends corner hits along both adjacent bands.
     * Left/top include the dividing stroke; right/bottom start after it. */
    top=p.y<l->corner.cy;bottom=p.y>=l->outer.bottom-l->corner.cy+l->unit;
    if(p.x<l->border)return top?HTTOPLEFT:bottom?HTBOTTOMLEFT:HTLEFT;
    if(p.x>=l->outer.right-l->border)return top?HTTOPRIGHT:bottom?HTBOTTOMRIGHT:HTRIGHT;
    return HTBORDER;
}
