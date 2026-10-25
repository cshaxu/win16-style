#include "updown_paint.h"
#include "vb_text.h"
#include <commctrl.h>
#include <vssym32.h>

static LONG enabled;
void ntw16s_updown_api_enable(void){InterlockedExchange(&enabled,1);}
BOOL ntw16s_updown_api_enabled(void){return InterlockedCompareExchange(&enabled,0,0)!=0;}
static _Thread_local ntw16s_updown_paint *windows;
static _Thread_local struct {HDC dc;ntw16s_updown_paint *window;} clones[32];
static _Thread_local unsigned clone_count;
void ntw16s_updown_register(ntw16s_updown_paint *s,HWND w,HMODULE native,const win16_style_palette *palette,
    ntw16s_updown_glyph *const *glyphs,void *owner,BOOL (*acquire)(void *),void (*release)(void *))
{
    s->window=w;s->native=native;s->palette=palette;s->glyphs=glyphs;s->owner=owner;s->acquire=acquire;s->release=release;
    s->next=windows;windows=s;
}
void ntw16s_updown_retire(ntw16s_updown_paint *s)
{
    if(!s->window)return;
    ntw16s_updown_paint **link=&windows;while(*link && *link!=s)link=&(*link)->next;
    if(*link)*link=s->next;
    for(unsigned i=0;i<32;++i)if(clones[i].window==s){clones[i].dc=NULL;clones[i].window=NULL;--clone_count;}
    s->window=NULL;s->next=NULL;
}
static ntw16s_updown_paint *target(HDC dc,void *caller,BOOL drawing)
{
    if(!windows || !dc)return NULL;
    HWND scope=NULL;BOOL supplied=ntw16s_native_paint_target(dc,&scope);
    HWND w=supplied?scope:WindowFromDC(dc);ntw16s_updown_paint *s=NULL;
    if(w){for(s=windows;s && s->window!=w;s=s->next){}}
    else if(clone_count){for(unsigned i=0;i<32;++i)if(clones[i].dc==dc){s=clones[i].window;break;}}
    if(!s || (drawing && scope && scope!=s->window) || !s->acquire(s->owner))return NULL;
    HMODULE module=NULL;
    DWORD style=(DWORD)GetWindowLongPtrW(s->window,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(s->window,GWL_EXSTYLE);
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (const WCHAR *)caller,&module) || module!=s->native || (drawing && (
        (style&0xffffu&~(UDS_WRAP|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ALIGNLEFT|UDS_AUTOBUDDY|UDS_ARROWKEYS|UDS_HORZ|UDS_NOTHOUSANDS|UDS_HOTTRACK)) ||
        (style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION|WS_HSCROLL|WS_VSCROLL)) ||
        (ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING))))){s->release(s->owner);return NULL;}
    return s;
}
void ntw16s_updown_dc_created(HDC source,HDC result,void *caller)
{
    if(!result)return;
    ntw16s_updown_paint *s=target(source,caller,FALSE);if(!s)return;
    for(unsigned i=0;i<32;++i)if(clones[i].dc==result || !clones[i].dc){
        if(!clones[i].dc)++clone_count;
        clones[i].dc=result;clones[i].window=s;break;
    }
    s->release(s->owner);
}
void ntw16s_updown_dc_deleted(HDC dc)
{
    if(clone_count)for(unsigned i=0;i<32;++i)if(clones[i].dc==dc){clones[i].dc=NULL;clones[i].window=NULL;--clone_count;}
}
static BOOL paint(ntw16s_updown_paint *s,HDC dc,const RECT *bounds,const RECT *clip,unsigned direction,BOOL pressed,BOOL disabled)
{
    RECT client,r=*bounds;if(!GetClientRect(s->window,&client) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom ||
        r.left>=r.right || r.top>=r.bottom || r.right>32767 || r.bottom>32767)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
    IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
    win16_style_palette p=*s->palette;COLORREF colors[4]={p.edge,p.light,p.shadow,p.face};
    HBRUSH brush=GetStockObject(DC_BRUSH);int unit=max(1,MulDiv(1,(int)GetDpiForWindow(s->window),96));
    SetDCBrushColor(dc,p.face);FillRect(dc,&r,brush);SetDCBrushColor(dc,p.edge);
    for(int i=0;i<unit && r.left<r.right && r.top<r.bottom;++i){FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);}
    if(r.left<r.right && r.top<r.bottom){
        RECT strip=r;SetDCBrushColor(dc,pressed?p.shadow:p.light);strip.bottom=min(r.top+unit,r.bottom);FillRect(dc,&strip,brush);
        strip=r;strip.right=min(r.left+unit,r.right);FillRect(dc,&strip,brush);
        SetDCBrushColor(dc,pressed?p.light:p.shadow);strip=r;strip.top=max(r.bottom-unit,r.top);FillRect(dc,&strip,brush);
        strip=r;strip.left=max(r.right-unit,r.left);FillRect(dc,&strip,brush);
        InflateRect(&r,-unit,-unit);
        int side=min(MulDiv(11,(int)GetDpiForWindow(s->window),96),min(r.right-r.left,r.bottom-r.top));
        if(side>0){
            /* The original arrow interior scales equally in both axes. The
             * native hit rectangle may be wide/short; never stretch the art. */
            int left=r.left+(r.right-r.left-side)/2,top=r.top+(r.bottom-r.top-side)/2;
            const char (*bits)[18]=*s->glyphs[direction+(disabled?8:pressed?4:0)];
            for(int y=0;y<11;++y)for(int x=0;x<11;++x){
                RECT pixel={left+MulDiv(x,side,11),top+MulDiv(y,side,11),left+MulDiv(x+1,side,11),top+MulDiv(y+1,side,11)};
                SetDCBrushColor(dc,colors[bits[y+3][x+3]-'0']);FillRect(dc,&pixel,brush);
            }
        }
    }
    RestoreDC(dc,saved);return TRUE;
}
BOOL ntw16s_updown_draw_frame(HDC dc,RECT *rect,UINT type,UINT state,void *caller)
{
    unsigned direction=state&15u;
    if(!rect || type!=DFC_SCROLL || direction>DFCS_SCROLLRIGHT ||
        (state&~(15u|DFCS_PUSHED|DFCS_INACTIVE|DFCS_HOT)) || ((state&DFCS_PUSHED) && (state&DFCS_INACTIVE)))return FALSE;
    ntw16s_updown_paint *s=target(dc,caller,TRUE);if(!s)return FALSE;
    BOOL result=paint(s,dc,rect,NULL,direction,(state&DFCS_PUSHED)!=0,(state&DFCS_INACTIVE)!=0);s->release(s->owner);return result;
}
BOOL ntw16s_updown_draw_theme(HTHEME theme,HDC dc,int part,int state,const RECT *rect,const RECT *clip,void *caller)
{
    (void)theme; /* Theme handles are shared, and cannot identify a window. */
    if(!rect || part<SPNP_UP || part>SPNP_DOWNHORZ || state<UPS_NORMAL || state>UPS_DISABLED)return FALSE;
    ntw16s_updown_paint *s=target(dc,caller,TRUE);if(!s)return FALSE;
    unsigned direction=part==SPNP_UP?0:part==SPNP_DOWN?1:part==SPNP_UPHORZ?3:2;
    BOOL result=paint(s,dc,rect,clip,direction,state==UPS_PRESSED,state==UPS_DISABLED);s->release(s->owner);return result;
}
