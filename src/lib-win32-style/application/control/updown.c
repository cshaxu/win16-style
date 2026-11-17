#include "updown.h"
#include <commctrl.h>
#include <vssym32.h>

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
