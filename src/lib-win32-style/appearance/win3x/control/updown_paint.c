#include "updown_paint.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/graphics/pixel_grid.h"
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
    BOOL painted=TRUE;
    if(s->art && s->art->source==STYLE_SCROLL_ARROW_SYMBOL){
        painted=style_paint_scroll_arrow(dc,&r,state,&p,s->art);
    }else{
        int dpi=(int)GetDpiForWindow(window),unit=max(1,MulDiv(1,dpi,96));
        r=style_paint_outlined_face(dc,r,unit,TRUE,pressed,p.face,&p);
        InflateRect(&r,-unit,-unit);
        enum { inset=3, image=STYLE_SCROLL_ARROW_SIDE-2*inset };
        int side=min(MulDiv(image,dpi,96),min(r.right-r.left,r.bottom-r.top));
        if(side>0){
            /* Center the square glyph inside the native button rectangle. */
            LONG left=r.left+(r.right-r.left-side)/2,top=r.top+(r.bottom-r.top-side)/2;
            RECT box={left,top,left+side,top+side};
            if(!style_paint_scroll_arrow_crop(dc,&box,state,&p,s->art)){
                const char (*bits)[18]=*s->glyphs[direction+(disabled?8:pressed?4:0)];
                const COLORREF colors[]={p.dark,p.highlight,p.shadow,p.face};
                const style_pixel_grid grid={&bits[inset][inset],image,image,sizeof(*bits),-1,colors,
                    {left,top},{side,side},{image,image}};
                style_paint_pixel_grid(dc,&grid);
            }
        }
    }
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
void win16_control_updown_register(win16_control_updown *s,style_control_updown *binding,HWND w,HMODULE native,const win16_style_palette *palette,ntw16s_updown_glyph *const *glyphs,void *owner,BOOL (*acquire)(void *),void (*release)(void *))
{
    s->palette=palette;s->glyphs=glyphs;
    const style_control_updown_ops ops={s,updown_supported,updown_render};
    style_control_updown_register(binding,w,native,owner,acquire,release,&ops);
}
