#include "progress_snapshot.h"
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
