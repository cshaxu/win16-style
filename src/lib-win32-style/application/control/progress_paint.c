#include "progress_paint.h"
#include "../graphics/paint_frame.h"

static BOOL paint_run(HDC dc,RECT lane,BOOL vertical,int start,int end,COLORREF color)
{
    if(vertical){
        lane.top=lane.bottom-end;
        lane.bottom-=start;
    }else{
        lane.right=lane.left+end;
        lane.left+=start;
    }
    return style_paint_color(dc,&lane,color);
}

BOOL style_progress_paint(HDC dc,const style_progress_snapshot *model,DWORD style,UINT dpi,
    const style_progress_layout *layout,const classic_progress_colors *colors,
    style_progress_ink ink,COLORREF disabled_fill)
{
    if(!dc || !layout || !(layout->flags&STYLE_PROGRESS_DEFINED) || !layout->block_whole)return FALSE;
    BOOL extra=colors && colors->extra_states;
    if(model->state!=PBST_NORMAL &&
       (!extra || (model->state!=PBST_ERROR && model->state!=PBST_PAUSED)))return FALSE;
    if(colors && colors->present){
        ink.background=colors->trough;
        ink.fill=colors->fill;
    }
    if(extra)ink.border=colors->border;
    if(model->background!=CLR_DEFAULT)ink.background=model->background;
    if(model->bar!=CLR_DEFAULT)ink.fill=model->bar;
    if(model->state==PBST_ERROR)ink.fill=colors->error;
    if(model->state==PBST_PAUSED)ink.fill=colors->paused;
    if(disabled_fill!=CLR_DEFAULT)ink.fill=disabled_fill;
    int unit=(layout->flags&STYLE_PROGRESS_SCALE_DPI)?max(1,MulDiv(1,(int)dpi,96)):1;
    int inset=unit*layout->inset;
    RECT lane=model->client;
    if(!style_paint_color(dc,&lane,ink.background))return FALSE;
    if(layout->flags&STYLE_PROGRESS_BORDER)
        style_paint_solid_frame(dc,lane,ink.border,inset);
    InflateRect(&lane,-inset,-inset);
    if(IsRectEmpty(&lane))return TRUE;

    BOOL vertical=(style&PBS_VERTICAL)!=0;
    int length=vertical?lane.bottom-lane.top:lane.right-lane.left;
    int thickness=vertical?lane.right-lane.left:lane.bottom-lane.top;
    int64_t span=(int64_t)model->range.iHigh-model->range.iLow;
    int64_t value=(int64_t)model->position-model->range.iLow;
    int64_t rounding=(layout->flags&STYLE_PROGRESS_ROUND)?span/2:0;
    int filled=span?(int)((value*length+rounding)/span):0;
    int limit=max(0,length-unit*layout->clearance);
    if(style&PBS_SMOOTH)return paint_run(dc,lane,vertical,0,min(filled,limit),ink.fill);
    int block=max(1,thickness*layout->block_part/layout->block_whole);
    int step=block+unit*layout->gap;
    int starts=min(filled,limit);
    int end=(layout->flags&STYLE_PROGRESS_FULL_BLOCKS)?limit:starts;
    for(int at=0;at<starts;at+=step)
        if(!paint_run(dc,lane,vertical,at,min(at+block,end),ink.fill))return FALSE;
    return TRUE;
}
