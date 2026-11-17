#include "scroll_layout.h"
#include <limits.h>
#include <stdint.h>

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

BOOL win2k_scroll_measure(const RECT *bar,BOOL vertical,int nominal,const SCROLLINFO *info,style_scroll_layout *out)
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
