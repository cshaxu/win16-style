#include "scroll_geometry.h"
#include <limits.h>
#include <stdint.h>
#include <string.h>

/* Independently expressed geometry from pinned USER 18:073C..081F;
 * original evidence and the modern-width extension are recorded in
 * docs/etc/evidence/m0-t445-s4-scrollbar-fidelity-audit.md.
 * This is not imported executable code or NT4's proportional-thumb branch. */
BOOL style_scroll_measure_fixed(const RECT *bar,BOOL vertical,
    const style_scroll_fixed_metrics *m,LONG minimum,LONG maximum,LONG position,
    style_scroll_layout *l)
{
    int64_t start,end,length,cross,arrow,origin,travel,thumb;
    int64_t range=(int64_t)maximum-minimum,offset;
    if(!bar || !m || !l || m->arrow<1 || m->thumb<1 || m->border<0 || range<0)return FALSE;
    start=vertical?bar->top:bar->left;end=vertical?bar->bottom:bar->right;
    length=end-start;cross=vertical?(int64_t)bar->right-bar->left:(int64_t)bar->bottom-bar->top;
    if(length<2*(int64_t)m->border || length<1 || length>INT_MAX || cross<1 || cross>INT_MAX)return FALSE;
    /* 18:07AA..07C7: shorten arrows, then subtract the shared border from
     * the minimum thumb origin. Preserve the border in both travel ends. */
    arrow=min(length/2-m->border,m->arrow);
    origin=start+arrow-m->border;
    travel=end-arrow-origin+m->border-m->thumb;
    if(origin<INT_MIN || origin>INT_MAX || travel<INT_MIN || travel>INT_MAX ||
        origin+max((int64_t)0,travel)+m->thumb>INT_MAX)return FALSE;
    position=max(minimum,min(maximum,position));offset=(int64_t)position-minimum;
    /* The original helper rounds multiplication/division. Extend only the
     * arithmetic width for native 32-bit ranges; no signed overflow/wrap. */
    thumb=origin;
    if(range && travel>0)thumb+=(offset*travel+range/2)/range;
    if(thumb<INT_MIN || thumb+m->thumb>INT_MAX)return FALSE;
    memset(l,0,sizeof(*l));l->bar=*bar;l->vertical=vertical;
    l->minimum=minimum;l->maximum=maximum;l->position=position;
    l->minimum_pixel=(int)origin;l->travel=(int)travel;
    l->first=l->last=l->thumb=*bar;
    /* USER 18:03DE..03EE skips the thumb if the region between arrows is
     * smaller than the fixed thumb; do not squeeze it into a modern thumb. */
    l->short_track_pages=TRUE;
    l->thumb_fits=length-2*arrow>=m->thumb;
    l->thumb_draggable=length-2*arrow>m->thumb;
    if(vertical){l->first.bottom=(LONG)(start+arrow);l->last.top=(LONG)(end-arrow);
        l->thumb.top=(LONG)thumb;l->thumb.bottom=(LONG)(thumb+m->thumb);}
    else {l->first.right=(LONG)(start+arrow);l->last.left=(LONG)(end-arrow);
        l->thumb.left=(LONG)thumb;l->thumb.right=(LONG)(thumb+m->thumb);}
    return TRUE;
}

style_scroll_part style_scroll_hit(const style_scroll_layout *l,POINT point,UINT disabled)
{
    LONG pixel,thumb_start,thumb_end;
    if(!l || !PtInRect(&l->bar,point) || (disabled&ESB_DISABLE_BOTH)==ESB_DISABLE_BOTH)
        return STYLE_SCROLL_NONE;
    /* USER 18:170B..1792 tests arrows before the overlapping thumb edge;
     * a thumb can be drawn in exactly enough room but cannot be dragged. */
    if(PtInRect(&l->first,point))return disabled&ESB_DISABLE_UP?STYLE_SCROLL_NONE:STYLE_SCROLL_LINE_BEFORE;
    if(PtInRect(&l->last,point))return disabled&ESB_DISABLE_DOWN?STYLE_SCROLL_NONE:STYLE_SCROLL_LINE_AFTER;
    if(!l->thumb_fits && !l->short_track_pages)return STYLE_SCROLL_NONE;
    pixel=l->vertical?point.y:point.x;
    thumb_start=l->vertical?l->thumb.top:l->thumb.left;
    thumb_end=l->vertical?l->thumb.bottom:l->thumb.right;
    if(pixel<thumb_start)return STYLE_SCROLL_PAGE_BEFORE;
    if(pixel>=thumb_end)return STYLE_SCROLL_PAGE_AFTER;
    return l->thumb_draggable?STYLE_SCROLL_THUMB:STYLE_SCROLL_NONE;
}

static RECT axis_rect(RECT bar,BOOL vertical,LONG first,LONG last)
{
    if(vertical){
        bar.top=first;
        bar.bottom=last;
    }else{
        bar.left=first;
        bar.right=last;
    }
    return bar;
}

BOOL style_scroll_measure_page(const RECT *bar,BOOL vertical,int nominal,const SCROLLINFO *info,style_scroll_layout *out)
{
    if(!bar || !info || !out || nominal<=0 || IsRectEmpty(bar) || info->nMax<info->nMin)return FALSE;
    LONG first=vertical?bar->top:bar->left;
    LONG last=vertical?bar->bottom:bar->right;
    int64_t length=(int64_t)last-first;
    if(length>INT_MAX)return FALSE;

    /* CalcSBStuff2 contracts from NT5: compressed arrows, a proportional
     * page thumb, and the same measured travel for drawing and tracking. */
    int arrow=min((int)length/2,nominal);
    int track=(int)length-2*arrow;
    uint64_t count=(uint64_t)((int64_t)info->nMax-info->nMin)+1;
    uint64_t page=min((uint64_t)info->nPage,count);
    int thumb=nominal;
    if(page){
        int proportional=(int)(((uint64_t)track*page+count/2)/count);
        thumb=max(max(1,nominal/2),proportional);
    }
    LONG origin=first+arrow;
    int travel=max(0,track-thumb);
    LONG maximum=(LONG)((int64_t)info->nMax-(int64_t)(page?page-1:0));
    LONG position=max(info->nMin,min(maximum,info->nPos));
    uint64_t span=(uint64_t)((int64_t)maximum-info->nMin);
    uint64_t relative=(uint64_t)((int64_t)position-info->nMin);
    LONG top=origin;
    if(span)top+=(LONG)((relative*(uint64_t)travel+span/2)/span);
    BOOL fits=track>=thumb;
    *out=(style_scroll_layout){
        .bar=*bar,
        .first=axis_rect(*bar,vertical,first,origin),
        .last=axis_rect(*bar,vertical,last-arrow,last),
        .thumb=axis_rect(*bar,vertical,top,top+(fits?thumb:0)),
        .minimum_pixel=origin,.travel=travel,
        .minimum=info->nMin,.maximum=maximum,.position=position,
        .vertical=vertical,.thumb_fits=fits,.thumb_draggable=fits && travel>0 && span>0
    };
    return TRUE;
}

BOOL style_scroll_measure(const RECT *bar,BOOL vertical,int nominal,int border,const SCROLLINFO *info,
    style_scroll_thumb thumb,style_scroll_layout *layout)
{
    if(thumb==STYLE_SCROLL_THUMB_PAGE)return style_scroll_measure_page(bar,vertical,nominal,info,layout);
    if(thumb!=STYLE_SCROLL_THUMB_FIXED || !info)return FALSE;
    style_scroll_fixed_metrics metrics={nominal,nominal,border};
    int64_t maximum=(int64_t)info->nMax-(info->nPage?info->nPage-1:0);
    if(maximum<info->nMin)maximum=info->nMin;
    return style_scroll_measure_fixed(bar,vertical,&metrics,info->nMin,(LONG)maximum,info->nPos,layout);
}
