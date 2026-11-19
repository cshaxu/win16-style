#include "updown_paint.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <commctrl.h>

BOOL win16_control_updown_paint(HWND window,const win16_style_palette *palette,ntw16s_updown_glyph *const *glyphs,HDC dc,const RECT *bounds,const RECT *clip,unsigned direction,BOOL pressed,BOOL disabled)
{
    RECT client,r=*bounds;if(!GetClientRect(window,&client) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom ||
        r.left>=r.right || r.top>=r.bottom || r.right>32767 || r.bottom>32767)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
    IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
    win16_style_palette p=*palette;COLORREF colors[4]={p.edge,p.light,p.shadow,p.face};
    HBRUSH brush=GetStockObject(DC_BRUSH);int unit=max(1,MulDiv(1,(int)GetDpiForWindow(window),96));
    SetDCBrushColor(dc,p.face);FillRect(dc,&r,brush);SetDCBrushColor(dc,p.edge);
    for(int i=0;i<unit && r.left<r.right && r.top<r.bottom;++i){FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);}
    if(r.left<r.right && r.top<r.bottom){
        RECT strip=r;SetDCBrushColor(dc,pressed?p.shadow:p.light);strip.bottom=min(r.top+unit,r.bottom);FillRect(dc,&strip,brush);
        strip=r;strip.right=min(r.left+unit,r.right);FillRect(dc,&strip,brush);
        SetDCBrushColor(dc,pressed?p.light:p.shadow);strip=r;strip.top=max(r.bottom-unit,r.top);FillRect(dc,&strip,brush);
        strip=r;strip.left=max(r.right-unit,r.left);FillRect(dc,&strip,brush);
        InflateRect(&r,-unit,-unit);
        int side=min(MulDiv(11,(int)GetDpiForWindow(window),96),min(r.right-r.left,r.bottom-r.top));
        if(side>0){
            /* The original arrow interior scales equally in both axes. The
             * native hit rectangle may be wide/short; never stretch the art. */
            int left=r.left+(r.right-r.left-side)/2,top=r.top+(r.bottom-r.top-side)/2;
            const char (*bits)[18]=*glyphs[direction+(disabled?8:pressed?4:0)];
            for(int y=0;y<11;++y)for(int x=0;x<11;++x){
                RECT pixel={left+MulDiv(x,side,11),top+MulDiv(y,side,11),left+MulDiv(x+1,side,11),top+MulDiv(y+1,side,11)};
                SetDCBrushColor(dc,colors[bits[y+3][x+3]-'0']);FillRect(dc,&pixel,brush);
            }
        }
    }
    RestoreDC(dc,saved);return TRUE;
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
    return win16_control_updown_paint(w,&palette,s->glyphs,dc,rect,clip,direction,pressed,disabled);
}
void win16_control_updown_register(win16_control_updown *s,style_control_updown *binding,HWND w,HMODULE native,const win16_style_palette *palette,ntw16s_updown_glyph *const *glyphs,void *owner,BOOL (*acquire)(void *),void (*release)(void *))
{
    s->palette=palette;s->glyphs=glyphs;
    const style_control_updown_ops ops={s,updown_supported,updown_render};
    style_control_updown_register(binding,w,native,owner,acquire,release,&ops);
}
