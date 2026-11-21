#include "updown_paint.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <commctrl.h>

static BOOL updown_paint(HWND window,const win16_style_palette *palette,const win16_control_updown *s,
    HDC dc,const RECT *bounds,const RECT *clip,unsigned direction,BOOL pressed,BOOL disabled)
{
    RECT client,r=*bounds;if(!GetClientRect(window,&client) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom ||
        r.left>=r.right || r.top>=r.bottom || r.right>32767 || r.bottom>32767)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
    IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
    const style_relief_palette p={.face=palette->face,.light=palette->light,.highlight=palette->light,
        .shadow=palette->shadow,.dark=palette->edge,.text=palette->edge};
    UINT state=direction|(pressed?DFCS_PUSHED:0)|(disabled?DFCS_INACTIVE:0);
    BOOL painted=style_paint_arrow_button(dc,r,state,GetDpiForWindow(window),&p,s->art,s->metrics);
    return RestoreDC(dc,saved) && painted;
}

static BOOL updown_supported(DWORD style,DWORD ex)
{
    return !(style&0xffffu&~(UDS_WRAP|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ALIGNLEFT|UDS_AUTOBUDDY|UDS_ARROWKEYS|UDS_HORZ|UDS_NOTHOUSANDS|UDS_HOTTRACK)) &&
        !(style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION|WS_HSCROLL|WS_VSCROLL)) && !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}
static BOOL updown_render(HWND w,void *context,HDC dc,const RECT *rect,const RECT *clip,unsigned direction,BOOL pressed,BOOL disabled)
{
    const win16_control_updown *s=context;
    win16_style_palette palette=*s->palette;
    if(s->frame && s->frame->present){
        palette.face=s->frame->face;
        palette.light=s->frame->light;
        palette.shadow=s->frame->shadow;
        palette.edge=s->frame->edge;
    }
    return updown_paint(w,&palette,s,dc,rect,clip,direction,pressed,disabled);
}
void win16_control_updown_register(win16_control_updown *s,style_control_updown *binding,HWND w,HMODULE native,const win16_style_palette *palette,void *owner,BOOL (*acquire)(void *),void (*release)(void *))
{
    s->palette=palette;
    const style_control_updown_ops ops={s,updown_supported,updown_render};
    style_control_updown_register(binding,w,native,owner,acquire,release,&ops);
}
