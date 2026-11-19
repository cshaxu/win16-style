#include "layout.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include <string.h>

BOOL win16_style_measure(int width,int height,UINT dpi,DWORD style,DWORD ex_style,
    BOOL maximized,win16_style_layout *l)
{
    static const classic_button_group group=WIN16_TITLE_BUTTON_GROUP;
    return win16_style_measure_with_buttons(width,height,dpi,style,ex_style,maximized,&group,NULL,l);
}

BOOL win16_style_measure_with_buttons(int width,int height,UINT dpi,DWORD style,DWORD ex_style,
    BOOL maximized,const classic_button_group *group,const classic_caption_metrics *metrics,
    win16_style_layout *l)
{
    int top,button,caption_height;BOOL caption=(style&WS_CAPTION)==WS_CAPTION;
    if(!l || width<1 || height<1 || dpi<48 || dpi>768)return FALSE;
    memset(l,0,sizeof(*l));
    button=MulDiv(metrics && metrics->button.cx?metrics->button.cx:19,(int)dpi,96);
    /* Round the one-pixel stroke like the scaled OEM separator, not down.
     * floor(dpi/96) left a one-column gap at 144 DPI. */
    l->unit=max(1,MulDiv(1,(int)dpi,96));
    l->sizeable=(style&WS_THICKFRAME)!=0 && !maximized;
    l->dialog_frame=(ex_style&WS_EX_DLGMODALFRAME)!=0;
    l->border=MulDiv(l->dialog_frame?6:l->sizeable?4:caption?3:
        (style&WS_DLGFRAME)?3:(style&WS_BORDER)?1:0,(int)dpi,96);
    top=l->border;
    /* Original VGA: 18-pixel artwork, 19-pixel button pitch. USER's
     * 20-pixel CYCAPTION includes both boundary lines, not 20 content rows. */
    /* Owner's unified20-pixel type must also fit on a96-DPI display. */
    caption_height=MulDiv(metrics && metrics->height?metrics->height:18,(int)dpi,96);
    if(caption)caption_height=max(20,caption_height);
    l->corner.cx=l->border+button;
    l->corner.cy=l->border+caption_height+l->unit;
    SetRect(&l->outer,0,0,width,height);
    if(!caption){
        SetRect(&l->client,l->border,l->border,max(l->border,width-l->border),
            max(l->border,height-l->border));
        return TRUE;
    }
    SetRect(&l->caption,l->border,top,max(l->border,width-l->border),top+caption_height);
    SetRect(&l->client,l->border,top+caption_height+l->unit,
        max(l->border,width-l->border),max(top+caption_height+l->unit,height-l->border));
    const unsigned mask=((style&WS_SYSMENU)?1u<<CLASSIC_BUTTON_SYSTEM:0) |
        ((style&WS_MINIMIZEBOX)?1u<<CLASSIC_BUTTON_MINIMIZE:0) |
        ((style&WS_MAXIMIZEBOX)?1u<<CLASSIC_BUTTON_MAXIMIZE:0);
    RECT slots[CLASSIC_BUTTON_CLOSE+1];
    if(!style_caption_slots(group,&l->caption,dpi,(SIZE){button,caption_height},mask,TRUE,slots))
        return FALSE;
    l->system=slots[CLASSIC_BUTTON_SYSTEM];
    l->minimize=slots[CLASSIC_BUTTON_MINIMIZE];
    l->maximize=slots[CLASSIC_BUTTON_MAXIMIZE];
    l->title=l->caption;
    for(unsigned i=0;i<group->count;++i){
        classic_button_slot slot=group->slots[i];
        if(!(mask&(1u<<slot.role)))continue;
        if(slot.side==CLASSIC_BUTTON_LEFT)l->title.left=max(l->title.left,slots[slot.role].right);
        else l->title.right=min(l->title.right,slots[slot.role].left);
    }
    l->title.right=max(l->title.left,l->title.right);
    return TRUE;
}

LRESULT win16_style_hit(const win16_style_layout *l,POINT p)
{
    BOOL left,right,top,bottom;
    if(!l || !PtInRect(&l->outer,p))return HTNOWHERE;
    if(PtInRect(&l->system,p))return HTSYSMENU;
    if(PtInRect(&l->minimize,p))return HTMINBUTTON;
    if(PtInRect(&l->maximize,p))return HTMAXBUTTON;
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
