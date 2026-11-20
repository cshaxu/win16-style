/* Windows 2000 comctl32 progress.c: ProPaint's physical inset, full blocks,
 * spacing and final clearance. Values and application colors come from the
 * shared native progress snapshot. */
#include "progress_paint.h"
#include "../common/paint.h"
#include <stdint.h>

BOOL win2k_progress_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    DWORD ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    return !(style&(0xffffu&~(PBS_SMOOTH|PBS_VERTICAL))) &&
        !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_WINDOWEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

static BOOL progress_run(HDC dc,RECT lane,BOOL vertical,int start,int end,COLORREF color)
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

BOOL win2k_progress_paint(HDC dc,const style_progress_snapshot *snapshot,DWORD style,const win2k_palette *palette)
{
    if(snapshot->state!=PBST_NORMAL)return FALSE;
    COLORREF background=snapshot->background==CLR_DEFAULT?palette->face:snapshot->background;
    if(!style_paint_color(dc,&snapshot->client,background))return FALSE;
    RECT lane=snapshot->client;
    InflateRect(&lane,-1,-1);
    if(IsRectEmpty(&lane))return TRUE;
    BOOL vertical=(style&PBS_VERTICAL)!=0;
    int length=vertical?lane.bottom-lane.top:lane.right-lane.left;
    int thickness=vertical?lane.right-lane.left:lane.bottom-lane.top;
    int64_t span=(int64_t)snapshot->range.iHigh-snapshot->range.iLow;
    int64_t value=(int64_t)snapshot->position-snapshot->range.iLow;
    int filled=span?(int)((value*length+span/2)/span):0;
    COLORREF ink=snapshot->bar==CLR_DEFAULT?palette->selection:snapshot->bar;
    int limit=length-1;
    if(style&PBS_SMOOTH)return progress_run(dc,lane,vertical,0,min(filled,limit),ink);
    int block=max(1,thickness*2/3);
    for(int at=0;at<filled;at+=block+2){
        if(!progress_run(dc,lane,vertical,at,min(at+block,limit),ink))return FALSE;
    }
    return TRUE;
}
