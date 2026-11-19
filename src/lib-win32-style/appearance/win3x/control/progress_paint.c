#include "progress_paint.h"
#include "control_state.h"
#include "../../../application/control/control_protocol.h"
#include <commctrl.h>
#include <stdint.h>
#include "lib-win32-style/application/control/progress_snapshot.h"

BOOL win16_style_progress_supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return style_control_protocol_is(w,PROGRESS_CLASSW) && !(style&0xffffu&~(PBS_SMOOTH|PBS_VERTICAL|PBS_SMOOTHREVERSE)) && !(style&(PBS_MARQUEE|WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

BOOL win16_style_progress_paint(HWND w,HDC dc,const control_style *control)
{
    if(!dc || control->instance.detached || !win16_style_progress_supported(w))return FALSE;
    style_progress_snapshot snapshot;
    if(!style_progress_snapshot_read(w,&control->instance,&control->bindings.progress,win16_style_progress_supported,&snapshot))
        return FALSE;
    RECT r=snapshot.client;PBRANGE range=snapshot.range;int position=snapshot.position;LRESULT state=snapshot.state;
    COLORREF bar=snapshot.bar,background=snapshot.background;
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    BOOL vertical=(style&PBS_VERTICAL)!=0;
    const win16_style_palette *palette=&control->palette;
    const classic_progress_colors *colors=&control->progress_colors[vertical];
    if(background==CLR_DEFAULT)background=colors->present?colors->trough:palette->light;
    if(bar==CLR_DEFAULT)bar=colors->present?colors->fill:palette->active;
    if(state==PBST_ERROR)bar=colors->extra_states?colors->error:RGB(192,0,0);
    if(state==PBST_PAUSED)bar=colors->extra_states?colors->paused:RGB(192,192,0);
    if(!IsWindowEnabled(w))bar=control->label_colors.present?control->label_colors.disabled:palette->shadow;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);SetDCBrushColor(dc,background);FillRect(dc,&r,brush);
    SetDCBrushColor(dc,colors->extra_states?colors->border:palette->edge);
    for(int i=0;i<u && r.right>r.left && r.bottom>r.top;++i){FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);}
    if(r.right>r.left && r.bottom>r.top){
        int length=vertical?r.bottom-r.top:r.right-r.left;
        int64_t span=(int64_t)range.iHigh-range.iLow;
        int filled=span?(int)(((int64_t)position-range.iLow)*length/span):0;
        SetDCBrushColor(dc,bar);
        if(style&PBS_SMOOTH){RECT fill=r;if(vertical)fill.top=fill.bottom-filled;else fill.right=fill.left+filled;FillRect(dc,&fill,brush);}
        else {int chunk=max(1,(vertical?r.right-r.left:r.bottom-r.top)*3/4),gap=2*u;
            for(int at=0;at<filled;at+=chunk+gap){RECT fill=r;int end=min(at+chunk,filled);
                if(vertical){fill.top=r.bottom-end;fill.bottom=r.bottom-at;}else{fill.left=r.left+at;fill.right=r.left+end;}
                FillRect(dc,&fill,brush);
            }
        }
    }
    RestoreDC(dc,saved);return TRUE;
}
