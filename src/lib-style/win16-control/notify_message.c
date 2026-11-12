#include "notify_message.h"
#include "../common/control_protocol.h"
#include "custom_draw.h"
#include "trackbar_paint.h"
#include "toolbar_paint.h"

LRESULT win16_control_trackbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,control_style *s,SUBCLASSPROC callback,track_paint_scope **active_scope)
{
    DWORD entry=GetLastError();LRESULT result;BOOL internal=FALSE;
    ++s->calls;NMCUSTOMDRAW *draw=NULL;
    if(m==WM_NOTIFY && l && !s->detached && GetParent(s->track_window)==parent){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==s->track_window && header->code==NM_CUSTOMDRAW)draw=(NMCUSTOMDRAW *)l;
    }
    if(draw && style_control_protocol_runtime_trackbar(s->track_window)){
        track_paint_scope *scope=*active_scope;
        BOOL match=scope && scope->owner==s && scope->active && scope->draw==draw && scope->dc==draw->hdc;
        if(match && draw->dwDrawStage==CDDS_ITEMPOSTPAINT && scope->pending && scope->item==draw->dwItemSpec){
            scope->pending=FALSE;
            if(EqualRect(&scope->rect,&draw->rc))win16_style_trackbar_runtime_border(s->track_window,draw,&s->palette);
            result=CDRF_DODEFAULT;
        }else if(match && draw->dwDrawStage==CDDS_POSTPAINT){scope->active=FALSE;result=CDRF_DODEFAULT;}
        else {
            SetLastError(entry);result=DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
            if(!s->detached && scope && *active_scope==scope && scope->owner==s){
                if(draw->dwDrawStage==CDDS_PREPAINT && result==CDRF_NOTIFYITEMDRAW &&
                   win16_style_trackbar_supported(s->track_window)){
                    scope->draw=draw;scope->dc=draw->hdc;scope->active=TRUE;
                    result=CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
                }else if(match && draw->dwDrawStage==CDDS_ITEMPREPAINT && result==CDRF_DODEFAULT && !scope->pending &&
                    (draw->dwItemSpec==TBCD_CHANNEL || draw->dwItemSpec==TBCD_THUMB)){
                    scope->item=draw->dwItemSpec;scope->rect=draw->rc;scope->pending=TRUE;result=CDRF_NOTIFYPOSTPAINT;
                }
            }
            if(!--s->calls && s->detached)win16_control_state_dispose(s);
            SetLastError(error);return result;
        }
        if(!--s->calls && s->detached)win16_control_state_dispose(s);
        SetLastError(entry);return result;
    }
    if(draw && draw->dwDrawStage!=CDDS_PREPAINT && s->track_internal)internal=TRUE;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(parent,callback,id);s->track_parent=NULL;s->track_internal=FALSE;}
    SetLastError(entry);
    result=internal?CDRF_DODEFAULT:DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
    if(draw && !s->detached)result=win16_control_custom_draw_after(s->track_window,draw,&s->track_internal,internal,result,
        &s->palette,win16_style_trackbar_supported,win16_style_trackbar_part);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    SetLastError(error);return result;
}

LRESULT win16_control_toolbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,control_style *s,SUBCLASSPROC callback)
{
    DWORD entry=GetLastError();LRESULT result;BOOL internal=FALSE;
    ++s->calls;NMCUSTOMDRAW *draw=NULL;
    if(m==WM_NOTIFY && l && !s->detached && GetParent(s->toolbar_window)==parent){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==s->toolbar_window && header->code==NM_CUSTOMDRAW)draw=(NMCUSTOMDRAW *)l;
    }
    if(draw && draw->dwDrawStage!=CDDS_PREPAINT && s->toolbar_internal)internal=TRUE;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(parent,callback,id);s->toolbar_parent=NULL;s->toolbar_internal=FALSE;}
    SetLastError(entry);result=internal?CDRF_DODEFAULT:DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
    if(draw && !s->detached)result=win16_control_custom_draw_after(s->toolbar_window,draw,&s->toolbar_internal,internal,result,
        &s->palette,win16_style_toolbar_supported,win16_style_toolbar_part);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    SetLastError(error);return result;
}
