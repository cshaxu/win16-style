#include "progress_paint.h"
#include "control_protocol.h"

static BOOL progress_window_supported(HWND window,DWORD allowed,DWORD blocked_edge)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    DWORD ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    return !(style&(0xffffu&~(PBS_SMOOTH|PBS_VERTICAL|allowed))) &&
        !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|blocked_edge|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

BOOL style_progress_bound_supported(HWND window)
{
    return progress_window_supported(window,PBS_SMOOTHREVERSE,WS_EX_STATICEDGE) &&
        style_control_protocol_is(window,PROGRESS_CLASSW);
}

BOOL style_progress_native_supported(HWND window)
{
    return progress_window_supported(window,0,WS_EX_WINDOWEDGE);
}

BOOL style_progress_present(HWND window,HDC dc,const style_instance *instance,
    const style_control_native_state *state,BOOL (*supported)(HWND),
    const style_progress_layout *layout,const classic_progress_colors colors[2],
    style_progress_ink fallback,COLORREF disabled_fill)
{
    style_progress_snapshot snapshot;
    if(!dc || instance->detached ||
        !style_progress_snapshot_read(window,instance,state,supported,&snapshot))return FALSE;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return style_progress_paint(dc,&snapshot,style,GetDpiForWindow(window),layout,
        &colors[(style&PBS_VERTICAL)!=0],fallback,IsWindowEnabled(window)?CLR_DEFAULT:disabled_fill);
}

BOOL style_progress_snapshot_read(HWND w,const style_instance *instance,const style_control_native_state *state,BOOL (*supported)(HWND),style_progress_snapshot *snapshot)
{
    *snapshot=(style_progress_snapshot){.epoch=state->epoch};
    RECT *r=&snapshot->client;
    if(!GetClientRect(w,r) || r->right<1 || r->bottom<1 || r->right>32767 || r->bottom>32767)return FALSE;
    SendMessageW(w,PBM_GETRANGE,0,(LPARAM)&snapshot->range);
    if(!style_control_native_current(w,instance,state,snapshot->epoch))return FALSE;
    snapshot->position=(int)(INT_PTR)SendMessageW(w,PBM_GETPOS,0,0);
    if(!style_control_native_current(w,instance,state,snapshot->epoch))return FALSE;
    snapshot->state=SendMessageW(w,PBM_GETSTATE,0,0);
    if(!style_control_native_current(w,instance,state,snapshot->epoch))return FALSE;
    snapshot->bar=(COLORREF)SendMessageW(w,PBM_GETBARCOLOR,0,0);
    if(!style_control_native_current(w,instance,state,snapshot->epoch))return FALSE;
    snapshot->background=(COLORREF)SendMessageW(w,PBM_GETBKCOLOR,0,0);
    return style_control_native_current(w,instance,state,snapshot->epoch) && supported(w) &&
        snapshot->range.iLow<=snapshot->range.iHigh && snapshot->position>=snapshot->range.iLow && snapshot->position<=snapshot->range.iHigh &&
        snapshot->state>=PBST_NORMAL && snapshot->state<=PBST_PAUSED &&
        (!(snapshot->bar&0xff000000u) || snapshot->bar==CLR_DEFAULT) &&
        (!(snapshot->background&0xff000000u) || snapshot->background==CLR_DEFAULT);
}
