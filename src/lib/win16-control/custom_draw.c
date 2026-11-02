#include "custom_draw.h"
LRESULT win16_control_custom_draw_after(HWND window,const NMCUSTOMDRAW *draw,BOOL *active,BOOL internal,
    LRESULT result,const win16_style_palette *palette,BOOL (*supported)(HWND),
    BOOL (*paint_part)(HWND,const NMCUSTOMDRAW *,const win16_style_palette *))
{
    if(draw->dwDrawStage==CDDS_PREPAINT){
        *active=result==CDRF_DODEFAULT && supported(window);
        if(*active)result=CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
    }else if(internal && draw->dwDrawStage==CDDS_ITEMPREPAINT && paint_part(window,draw,palette))result=CDRF_SKIPDEFAULT;
    else if(internal && draw->dwDrawStage==CDDS_POSTPAINT)*active=FALSE;
    return result;
}
