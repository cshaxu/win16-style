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

LRESULT style_control_custom_draw_post(HWND parent,UINT message,WPARAM wp,LPARAM lp,
    void *owner,HWND window,BOOL *active,BOOL *painted,BOOL (*valid)(HWND,void *),
    BOOL (*paint)(HWND,void *,const NMCUSTOMDRAW *))
{
    if(message!=WM_NOTIFY || !lp || !active || window!=((NMHDR *)lp)->hwndFrom ||
       ((NMHDR *)lp)->code!=NM_CUSTOMDRAW || !valid(window,owner))
        return DefSubclassProc(parent,message,wp,lp);
    NMCUSTOMDRAW *draw=(NMCUSTOMDRAW *)lp;
    if(draw->dwDrawStage==CDDS_PREPAINT){
        LRESULT result=DefSubclassProc(parent,message,wp,lp);
        *active=result==CDRF_DODEFAULT && valid(window,owner);
        return *active?CDRF_NOTIFYITEMDRAW:result;
    }
    if(!*active || !valid(window,owner))return DefSubclassProc(parent,message,wp,lp);
    if(draw->dwDrawStage==CDDS_ITEMPREPAINT)return CDRF_NOTIFYPOSTPAINT;
    if(draw->dwDrawStage==CDDS_ITEMPOSTPAINT){
        if(paint(window,owner,draw))*painted=TRUE;
        return CDRF_DODEFAULT;
    }
    return DefSubclassProc(parent,message,wp,lp);
}
