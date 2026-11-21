#include "updown.h"
#include "native.h"
#include "../adapters/win32/runtime_text.h"
#include "../lifecycle/dispatch.h"
#include <commctrl.h>
#include <vssym32.h>
#include "../graphics/bitmap.h"

static LONG enabled;
void style_control_updown_api_enable(void){InterlockedExchange(&enabled,1);}
BOOL style_control_updown_api_enabled(void){return InterlockedCompareExchange(&enabled,0,0)!=0;}
static _Thread_local style_control_updown *windows;
static _Thread_local struct {HDC dc;style_control_updown *window;} clones[32];
static _Thread_local unsigned clone_count;
void style_control_updown_register(style_control_updown *s,HWND w,HMODULE native,void *owner,BOOL (*acquire)(void *),void (*release)(void *),const style_control_updown_ops *ops)
{
    s->window=w;s->native=native;s->owner=owner;s->acquire=acquire;s->release=release;s->ops=*ops;
    s->next=windows;windows=s;
}
static BOOL indexed_styles(DWORD style,DWORD ex)
{
    DWORD allowed=UDS_WRAP|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ALIGNLEFT|UDS_AUTOBUDDY|UDS_ARROWKEYS|
        UDS_HORZ|UDS_NOTHOUSANDS|UDS_HOTTRACK;
    return !(style&(0xffffu&~allowed)) && !(style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}
BOOL style_control_updown_indexed_supported(HWND window)
{
    return indexed_styles((DWORD)GetWindowLongPtrW(window,GWL_STYLE),(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE));
}
static void indexed_paint_end(void *context,BOOL completed)
{(void)completed;style_runtime_text_end(context);}
LRESULT style_control_updown_indexed_message(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message!=WM_PAINT && message!=WM_PRINTCLIENT && message!=WM_PRINT && message!=WM_ERASEBKGND)
        return DefSubclassProc(window,message,a,b);
    style_runtime_text_scope scope;
    style_runtime_native_paint_begin(&scope,window,message,message==WM_PAINT?NULL:(HDC)a);
    const style_dispatch_ops ops={NULL,&scope,NULL,indexed_paint_end};
    return style_dispatch_message(window,message,a,b,&ops);
}
static BOOL indexed_paint(HWND window,void *context,HDC dc,const RECT *bounds,const RECT *clip,
    unsigned direction,BOOL pressed,BOOL disabled)
{
    const style_control_updown_art *model=context;
    RECT client,r=*bounds;
    if(!GetClientRect(window,&client) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom ||
        IsRectEmpty(&r) || r.right>32767 || r.bottom>32767)return FALSE;
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
    IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
    UINT state=direction|(pressed?DFCS_PUSHED:0)|(disabled?DFCS_INACTIVE:0);
    BOOL painted=style_paint_arrow_button(dc,r,state,GetDpiForWindow(window),&model->palette,model->art,model->metrics);
    return RestoreDC(dc,saved) && painted;
}
void style_control_updown_bind_art(style_control_updown *binding,HWND window,HMODULE native,
    style_control_updown_art *model,void *owner,BOOL (*acquire)(void *),void (*release)(void *))
{
    const style_control_updown_ops ops={model,indexed_styles,indexed_paint};
    style_control_updown_register(binding,window,native,owner,acquire,release,&ops);
}
void style_control_updown_retire(style_control_updown *s)
{
    if(!s->window)return;
    style_control_updown **link=&windows;while(*link && *link!=s)link=&(*link)->next;
    if(*link)*link=s->next;
    for(unsigned i=0;i<32;++i)if(clones[i].window==s){clones[i].dc=NULL;clones[i].window=NULL;--clone_count;}
    s->window=NULL;s->next=NULL;
}
static style_control_updown *target(HDC dc,const void *caller,BOOL drawing,BOOL (*resolve)(HDC,HWND *))
{
    if(!windows || !dc)return NULL;
    HWND scope=NULL;BOOL supplied=resolve(dc,&scope);
    HWND w=supplied?scope:WindowFromDC(dc);style_control_updown *s=NULL;
    if(w){for(s=windows;s && s->window!=w;s=s->next){}}
    else if(clone_count){for(unsigned i=0;i<32;++i)if(clones[i].dc==dc){s=clones[i].window;break;}}
    if(!s || (drawing && scope && scope!=s->window) || !s->acquire(s->owner))return NULL;
    HMODULE module=NULL;
    DWORD style=(DWORD)GetWindowLongPtrW(s->window,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(s->window,GWL_EXSTYLE);
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (const WCHAR *)caller,&module) || module!=s->native || (drawing && !s->ops.supported(style,ex))){s->release(s->owner);return NULL;}
    return s;
}
void style_control_updown_dc_created(HDC source,HDC result,const void *caller,BOOL (*resolve)(HDC,HWND *))
{
    if(!result)return;
    style_control_updown *s=target(source,caller,FALSE,resolve);if(!s)return;
    for(unsigned i=0;i<32;++i)if(clones[i].dc==result || !clones[i].dc){
        if(!clones[i].dc)++clone_count;
        clones[i].dc=result;clones[i].window=s;break;
    }
    s->release(s->owner);
}
void style_control_updown_dc_deleted(HDC dc)
{
    if(clone_count)for(unsigned i=0;i<32;++i)if(clones[i].dc==dc){clones[i].dc=NULL;clones[i].window=NULL;--clone_count;}
}
BOOL style_control_updown_draw_frame(HDC dc,RECT *rect,UINT type,UINT state,const void *caller,BOOL (*resolve)(HDC,HWND *))
{
    unsigned direction=state&15u;
    if(!rect || type!=DFC_SCROLL || direction>DFCS_SCROLLRIGHT ||
        (state&~(15u|DFCS_PUSHED|DFCS_INACTIVE|DFCS_HOT)) || ((state&DFCS_PUSHED) && (state&DFCS_INACTIVE)))return FALSE;
    style_control_updown *s=target(dc,caller,TRUE,resolve);if(!s)return FALSE;
    BOOL result=s->ops.render(s->window,s->ops.context,dc,rect,NULL,direction,(state&DFCS_PUSHED)!=0,(state&DFCS_INACTIVE)!=0);s->release(s->owner);return result;
}
BOOL style_control_updown_draw_theme(HTHEME theme,HDC dc,int part,int state,const RECT *rect,const RECT *clip,const void *caller,BOOL (*resolve)(HDC,HWND *))
{
    (void)theme; /* Theme handles are shared, and cannot identify a window. */
    if(!rect || part<SPNP_UP || part>SPNP_DOWNHORZ || state<UPS_NORMAL || state>UPS_DISABLED)return FALSE;
    style_control_updown *s=target(dc,caller,TRUE,resolve);if(!s)return FALSE;
    unsigned direction=part==SPNP_UP?0:part==SPNP_DOWN?1:part==SPNP_UPHORZ?3:2;
    BOOL result=s->ops.render(s->window,s->ops.context,dc,rect,clip,direction,state==UPS_PRESSED,state==UPS_DISABLED);s->release(s->owner);return result;
}

/* Preserve the native scroll-arrow frames and map their palette roles.
 * An odd split can leave a one-pixel unpainted seam. */
BOOL style_control_updown_recolor_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    DWORD allowed=UDS_WRAP|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ALIGNLEFT|UDS_AUTOBUDDY|UDS_ARROWKEYS|
        UDS_HORZ|UDS_NOTHOUSANDS|UDS_HOTTRACK;
    return !(style&(0xffffu&~allowed)) && !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

static DWORD dib_color(COLORREF color)
{
    return ((DWORD)GetRValue(color)<<16)|((DWORD)GetGValue(color)<<8)|GetBValue(color);
}

static BOOL recolor(HWND window,style_instance *instance,HDC target,const style_relief_palette *p)
{
    const COLORREF key=RGB(253,11,239);
    const int indices[]={COLOR_BTNFACE,COLOR_3DLIGHT,COLOR_3DSHADOW,COLOR_3DDKSHADOW,COLOR_3DHIGHLIGHT,COLOR_BTNTEXT};
    const COLORREF colors[]={p->face,p->light,p->shadow,p->dark,p->highlight,p->text};
    DWORD input[ARRAYSIZE(indices)],output[ARRAYSIZE(indices)];
    for(unsigned i=0;i<ARRAYSIZE(indices);++i){
        input[i]=dib_color(GetSysColor(indices[i]));
        output[i]=dib_color(colors[i]);
        if(input[i]==dib_color(key))return FALSE;
        for(unsigned j=0;j<i;++j)if(input[i]==input[j] && output[i]!=output[j])return FALSE;
    }
    RECT client;
    if(!target || !GetClientRect(window,&client) || client.right<1 || client.bottom<1 ||
        client.right>512 || client.bottom>512)return FALSE;
    style_bitmap surface={0};
    SIZE size={client.right,client.bottom};
    if(!style_bitmap_ensure(&surface,target,size))return FALSE;
    BOOL painted=FALSE;
    if(SetDCBrushColor(surface.dc,key)==CLR_INVALID || !FillRect(surface.dc,&client,GetStockObject(DC_BRUSH)))goto done;
    DefSubclassProc(window,WM_PRINTCLIENT,(WPARAM)surface.dc,PRF_CLIENT);
    if(instance->detached || !GdiFlush())goto done;
    DWORD *pixels=(DWORD *)surface.pixels;
    size_t count=(size_t)size.cx*(size_t)size.cy;
    BOOL mapped=FALSE;
    for(size_t n=0;n<count;++n){
        DWORD pixel=pixels[n]&0xffffffu;
        for(unsigned i=0;i<ARRAYSIZE(input);++i){
            if(pixel==input[i]){pixels[n]=output[i];mapped=TRUE;break;}
        }
    }
    if(mapped)painted=TransparentBlt(target,0,0,size.cx,size.cy,surface.dc,0,0,size.cx,size.cy,key);
done:
    style_bitmap_dispose(&surface);
    return painted;
}

typedef struct updown_recolor {
    style_instance *instance;
    const style_relief_palette *palette;
} updown_recolor;
static void recolor_render(HWND window,void *context,HDC dc,BOOL buffered)
{
    (void)buffered;
    const updown_recolor *paint=context;
    if(recolor(window,paint->instance,dc,paint->palette) || paint->instance->detached)return;
    int saved=SaveDC(dc);
    if(saved){DefSubclassProc(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);RestoreDC(dc,saved);}
}
LRESULT style_control_updown_message(HWND window,style_instance *instance,UINT message,WPARAM a,LPARAM b,
    const style_relief_palette *palette)
{
    updown_recolor paint={instance,palette};
    const style_control_draw draw={&paint,recolor_render};
    return style_control_native_message(window,instance,NULL,STYLE_CONTROL_UPDOWN,
        message,a,b,style_control_updown_recolor_supported,&draw);
}
