#include "scroll_track.h"
#include "lib-win32-style/application/control/scroll_state.h"
#include <stdint.h>
#include <string.h>

/* Source observations: pinned Win3.1 USER18:110C,1369,1447,14E5,1636.
 * See the scrollbar audit. No imported code, global state or modal loop. */
static UINT command(style_scroll_part part)
{
    switch(part){
    case STYLE_SCROLL_LINE_BEFORE:return SB_LINEUP;
    case STYLE_SCROLL_LINE_AFTER:return SB_LINEDOWN;
    case STYLE_SCROLL_PAGE_BEFORE:return SB_PAGEUP;
    case STYLE_SCROLL_PAGE_AFTER:return SB_PAGEDOWN;
    default:return SB_THUMBTRACK;
    }
}

void style_scroll_repeat_configure(style_scroll_policy *policy,UINT initial_ms,UINT repeat_ms)
{
    if(!initial_ms){
        UINT system=GetDoubleClickTime()*4/5;
        initial_ms=max(1,system);
        repeat_ms=max(1,system/8);
    }
    policy->initial_repeat_ms=initial_ms;
    policy->repeat_ms=repeat_ms;
}

BOOL style_scroll_begin(style_scroll_track *t,const style_scroll_layout *l,
    UINT disabled,POINT point,const RECT *limit,style_scroll_actions *a,const style_scroll_policy *policy)
{
    style_scroll_part part;
    if(!t || !l || !limit || !a)return FALSE;
    memset(a,0,sizeof(*a));if(t->active)return FALSE;
    part=policy->hit(l,point,disabled);if(part==STYLE_SCROLL_NONE)return FALSE;
    memset(t,0,sizeof(*t));t->part=part;t->drag_limit=*limit;
    t->initial_pixel=t->pixel=l->vertical?l->thumb.top:l->thumb.left;
    if(part==STYLE_SCROLL_THUMB)t->grab_offset=(l->vertical?point.y:point.x)-t->initial_pixel;
    t->position=l->position;t->active=t->inside=TRUE;t->repeat_ms=policy->initial_repeat_ms;
    a->repaint=TRUE;a->count=1;a->notices[0].command=command(part);
    if(part==STYLE_SCROLL_THUMB)a->notices[0].position=t->position;
    else a->timer_ms=t->repeat_ms;
    return TRUE;
}

void style_scroll_motion(style_scroll_track *t,const style_scroll_layout *l,
    UINT disabled,POINT point,BOOL timer,style_scroll_actions *a,const style_scroll_policy *policy)
{
    BOOL inside;int pixel;int64_t desired;
    memset(a,0,sizeof(*a));a->timer_ms=STYLE_SCROLL_KEEP_TIMER;if(!t->active)return;
    if(t->part==STYLE_SCROLL_THUMB){
        /* Original outside-drag-area behavior snaps to the initial pixel;
         * moving back in resumes using the original pointer grab offset. */
        if(timer)return;
        inside=PtInRect(&t->drag_limit,point);
        desired=(int64_t)(l->vertical?point.y:point.x)-t->grab_offset;
        if(inside)pixel=(int)max((int64_t)l->minimum_pixel,
            min((int64_t)l->minimum_pixel+l->travel,desired));
        else pixel=t->initial_pixel;
        if(pixel!=t->pixel){
            LONG position=style_scroll_position(l->minimum_pixel,l->travel,l->minimum,l->maximum,pixel);
            a->repaint=TRUE;t->pixel=pixel;
            if(position!=t->position){
                t->position=position;a->count=1;
                a->notices[0].command=SB_THUMBTRACK;a->notices[0].position=position;
            }
        }
        t->inside=inside;return;
    }
    inside=policy->hit(l,point,disabled)==t->part;
    a->repaint=inside!=t->inside;
    if(timer)t->repeat_ms=policy->repeat_ms;
    if(inside && (timer || !t->inside)){
        a->count=1;a->notices[0].command=command(t->part);
    }
    if(timer || inside!=t->inside)a->timer_ms=inside?t->repeat_ms:0;
    t->inside=inside;
}

void style_scroll_end(style_scroll_track *t,BOOL cancel,style_scroll_actions *a)
{
    memset(a,0,sizeof(*a));if(!t->active)return;
    /* Commit termination before caller notification: callbacks may remove,
     * destroy or replace the attachment and must never see it still active. */
    t->active=FALSE;t->inside=FALSE;a->repaint=TRUE;
    if(t->part==STYLE_SCROLL_THUMB){
        if(cancel)t->pixel=t->initial_pixel;
        /* USER18:1399..13C1 restores the visual thumb on cancel but reports
         * the last notified position (04FE); it does not invent a new
         * SB_THUMBTRACK back to the starting application position. */
        a->notices[a->count].command=SB_THUMBPOSITION;
        a->notices[a->count++].position=t->position;
    }
    a->notices[a->count++].command=SB_ENDSCROLL;
}
