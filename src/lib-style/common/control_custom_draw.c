#include "control_custom_draw.h"
LRESULT style_control_custom_draw_after(HWND window,const NMCUSTOMDRAW *draw,BOOL *active,BOOL internal,
    LRESULT result,const void *context,BOOL (*supported)(HWND),
    BOOL (*paint_part)(HWND,const NMCUSTOMDRAW *,const void *))
{
    if(draw->dwDrawStage==CDDS_PREPAINT){
        *active=result==CDRF_DODEFAULT && supported(window);
        if(*active)result=CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
    }else if(internal && draw->dwDrawStage==CDDS_ITEMPREPAINT && paint_part(window,draw,context))result=CDRF_SKIPDEFAULT;
    else if(internal && draw->dwDrawStage==CDDS_POSTPAINT)*active=FALSE;
    return result;
}
