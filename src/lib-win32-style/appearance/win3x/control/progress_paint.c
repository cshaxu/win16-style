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
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    const win16_style_palette *palette=&control->palette;
    const classic_progress_colors *colors=&control->progress_colors[(style&PBS_VERTICAL)!=0];
    COLORREF disabled_fill=IsWindowEnabled(w)?CLR_DEFAULT:
        control->label_colors.present?control->label_colors.disabled:palette->shadow;
    return style_progress_paint(dc,&snapshot,style,GetDpiForWindow(w),&control->controls.progress,
        colors,(style_progress_ink){palette->light,palette->active,palette->edge},disabled_fill);
}
