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
BOOL ntw16s_updown_draw_frame(HDC dc,RECT *rect,UINT type,UINT state,void *caller)
{
    unsigned direction=state&15u;
    if(!rect || type!=DFC_SCROLL || direction>DFCS_SCROLLRIGHT ||
        (state&~(15u|DFCS_PUSHED|DFCS_INACTIVE|DFCS_HOT)) || ((state&DFCS_PUSHED) && (state&DFCS_INACTIVE)))return FALSE;
    ntw16s_updown_paint *s=target(dc,caller,TRUE);if(!s)return FALSE;
    BOOL result=win16_control_updown_paint(s->window,s->palette,s->glyphs,dc,rect,NULL,direction,(state&DFCS_PUSHED)!=0,(state&DFCS_INACTIVE)!=0);s->release(s->owner);return result;
}
BOOL ntw16s_updown_draw_theme(HTHEME theme,HDC dc,int part,int state,const RECT *rect,const RECT *clip,void *caller)
{
    (void)theme; /* Theme handles are shared, and cannot identify a window. */
    if(!rect || part<SPNP_UP || part>SPNP_DOWNHORZ || state<UPS_NORMAL || state>UPS_DISABLED)return FALSE;
    ntw16s_updown_paint *s=target(dc,caller,TRUE);if(!s)return FALSE;
    unsigned direction=part==SPNP_UP?0:part==SPNP_DOWN?1:part==SPNP_UPHORZ?3:2;
    BOOL result=win16_control_updown_paint(s->window,s->palette,s->glyphs,dc,rect,clip,direction,state==UPS_PRESSED,state==UPS_DISABLED);s->release(s->owner);return result;
}
