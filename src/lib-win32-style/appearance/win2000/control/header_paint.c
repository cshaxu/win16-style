/* Windows 2000 comctl32 header.c: retain the native custom-draw transaction. */
#include "tab_header.h"
#include "../common/palette.h"
#include "lib-win32-style/application/control/header_snapshot.h"

BOOL win2k_header_supported(HWND window)
{return style_header_supported(window,HDS_HOTTRACK);}

BOOL win2k_header_paint(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    const win2k_header_draw *parameters=context;
    if(!parameters->profile)return FALSE;
    const style_instance *instance=parameters->instance;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    const style_relief_palette *p=&parameters->palette;
    WCHAR text[256]={0};
    HDITEMW item={.mask=HDI_FORMAT,.pszText=text,.cchTextMax=ARRAYSIZE(text)};
    LRESULT index=(LRESULT)draw->dwItemSpec;
    if(index<0 || !SendMessageW(window,HDM_GETITEMW,index,(LPARAM)&item) || instance->detached ||
       (item.fmt&~(HDF_STRING|HDF_JUSTIFYMASK|HDF_RTLREADING)))return FALSE;
    item.mask=HDI_TEXT;
    if((item.fmt&HDF_STRING) && (!SendMessageW(window,HDM_GETITEMW,index,(LPARAM)&item) || instance->detached ||
       item.pszText!=text || text[ARRAYSIZE(text)-2]))return FALSE;
    HDC dc=draw->hdc;
    RECT r=draw->rc;
    if(IsRectEmpty(&r))return TRUE;
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    BOOL pressed=(style&HDS_BUTTONS) && (draw->uItemState&CDIS_SELECTED);
    const style_header_colors *colors=parameters->colors;
    COLORREF face=colors->present?(pressed?colors->pressed:colors->normal):p->face;
    int unit=max(1,MulDiv(1,(int)GetDpiForWindow(window),96));
    BOOL painted=IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR &&
        style_paint_color(dc,&r,face) &&
        style_paint_header_edge(dc,r,(style&HDS_BUTTONS)!=0,(style&WS_BORDER)!=0,pressed,unit,parameters->profile,p);
    if(painted && *text){
        InflateRect(&r,-6,0);
        if(pressed)OffsetRect(&r,1,1);
        style_header_draw_text(dc,r,text,item.fmt,IsWindowEnabled(window)?colors:NULL,pressed,p->text);
    }
    return RestoreDC(dc,saved) && painted;
}
