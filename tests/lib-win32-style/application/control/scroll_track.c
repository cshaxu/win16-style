#include "lib-win32-style/application/control/scroll_track.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TRACK line=%d\n",__LINE__);return 1;}}while(0)
static style_scroll_part hit(const style_scroll_layout *layout,POINT point,UINT disabled)
{
    if(disabled==ESB_DISABLE_BOTH || !PtInRect(&layout->bar,point))return STYLE_SCROLL_NONE;
    if(PtInRect(&layout->first,point))return STYLE_SCROLL_LINE_BEFORE;
    if(PtInRect(&layout->thumb,point))return STYLE_SCROLL_THUMB;
    return STYLE_SCROLL_PAGE_AFTER;
}
static style_scroll_part page_hit(const style_scroll_layout *layout,POINT point,UINT disabled)
{return hit(layout,point,disabled)==STYLE_SCROLL_NONE?STYLE_SCROLL_NONE:STYLE_SCROLL_PAGE_BEFORE;}
int main(void)
{
    style_scroll_layout layout={.bar={0,0,20,200},.first={0,0,20,20},.last={0,180,20,200},.thumb={0,80,20,100},
        .minimum_pixel=20,.travel=120,.minimum=0,.maximum=100,.position=50,.vertical=TRUE,.thumb_fits=TRUE,.thumb_draggable=TRUE};
    style_scroll_policy policy={150,27,{STYLE_SCROLL_DRAG_LOGICAL,6,2},hit},other={310,41,{STYLE_SCROLL_DRAG_LOGICAL,1,1},page_hit};
    style_scroll_track track={0};style_scroll_actions actions;RECT limit={-6,-2,26,202};POINT point={10,10};
    CHECK(style_scroll_begin(&track,&layout,0,point,&limit,&actions,&policy));
    CHECK(track.active && actions.count==1 && actions.notices[0].command==SB_LINEUP && actions.timer_ms==150);
    CHECK(!style_scroll_begin(&track,&layout,0,point,&limit,&actions,&policy) && track.active);
    style_scroll_motion(&track,&layout,0,point,TRUE,&actions,&policy);
    CHECK(actions.count==1 && actions.timer_ms==27 && track.repeat_ms==27);
    point.x=40;style_scroll_motion(&track,&layout,0,point,FALSE,&actions,&policy);
    CHECK(!track.inside && !actions.count && !actions.timer_ms);
    style_scroll_end(&track,FALSE,&actions);CHECK(!track.active && actions.count==1 && actions.notices[0].command==SB_ENDSCROLL);
    point=(POINT){10,10};CHECK(style_scroll_begin(&track,&layout,0,point,&limit,&actions,&other));
    CHECK(actions.notices[0].command==SB_PAGEUP && actions.timer_ms==310);
    style_scroll_motion(&track,&layout,0,point,TRUE,&actions,&other);CHECK(actions.timer_ms==41);
    style_scroll_end(&track,FALSE,&actions);
    point=(POINT){10,85};CHECK(style_scroll_begin(&track,&layout,0,point,&limit,&actions,&policy));
    CHECK(track.grab_offset==5 && track.position==50 && !actions.timer_ms);
    point.y=125;style_scroll_motion(&track,&layout,0,point,FALSE,&actions,&policy);
    CHECK(track.pixel==120 && track.position==83 && actions.count==1 && actions.notices[0].command==SB_THUMBTRACK);
    style_scroll_end(&track,TRUE,&actions);
    CHECK(!track.active && track.pixel==80 && actions.count==2 && actions.notices[0].command==SB_THUMBPOSITION && actions.notices[0].position==83 && actions.notices[1].command==SB_ENDSCROLL);
    style_scroll_end(&track,TRUE,&actions);CHECK(!actions.count);
    CHECK(!style_scroll_begin(&track,&layout,ESB_DISABLE_BOTH,point,&limit,&actions,&policy));
    puts("TRACK independent geometry/hit policy, two repeat policies, thumb/cancel notice contract PASS");return 0;
}
