#include "progress_paint.h"
#include "control_state.h"
#include "../../../application/control/control_protocol.h"
#include <commctrl.h>
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
    LRESULT state=snapshot.state;
    COLORREF bar=snapshot.bar,background=snapshot.background;
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    BOOL vertical=(style&PBS_VERTICAL)!=0;
    const win16_style_palette *palette=&control->palette;
    const classic_progress_colors *colors=&control->progress_colors[vertical];
    if(background==CLR_DEFAULT)background=colors->present?colors->trough:palette->light;
    if(bar==CLR_DEFAULT)bar=colors->present?colors->fill:palette->active;
    if(state==PBST_ERROR)bar=colors->extra_states?colors->error:RGB(192,0,0);
    if(state==PBST_PAUSED)bar=colors->extra_states?colors->paused:RGB(192,192,0);
    if(!IsWindowEnabled(w))bar=control->label_colors.present?control->label_colors.disabled:palette->shadow;
    static const style_progress_layout fallback=STYLE_PROGRESS_OUTLINED;
    const style_progress_layout *layout=control->controls.progress.flags?&control->controls.progress:&fallback;
    const style_progress_ink ink={background,bar,colors->extra_states?colors->border:palette->edge};
    return style_progress_paint(dc,&snapshot,style,GetDpiForWindow(w),layout,ink);
}
