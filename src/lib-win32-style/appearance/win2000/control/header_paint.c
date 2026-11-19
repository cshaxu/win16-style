/* Windows 2000 comctl32 header.c: retain the native custom-draw transaction. */
#include "tab_header.h"
#include "../common/paint.h"

BOOL win2k_header_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return !(style&(HDS_FILTERBAR|HDS_CHECKBOXES|HDS_OVERFLOW|HDS_HIDDEN|HDS_HOTTRACK));
}

BOOL win2k_header_paint(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    const win2k_header_draw *parameters=context;
    const style_instance *instance=parameters->instance;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    const win2k_palette *p=&win2k_standard_palette;
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
    const classic_header_colors *colors=parameters->colors;
    COLORREF face=colors->present?(pressed?colors->pressed:colors->normal):p->face;
    UINT sides=style&HDS_BUTTONS?BF_RECT|BF_SOFT:(style&WS_BORDER?BF_RIGHT:BF_BOTTOMRIGHT);
    UINT edge=style&HDS_BUTTONS?(pressed?EDGE_SUNKEN:EDGE_RAISED):EDGE_ETCHED;
    BOOL painted=IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR &&
        win2k_fill(dc,&r,face) && win2k_edge(dc,&r,edge,sides|(pressed?BF_FLAT:0),p);
    if(painted && *text){
        InflateRect(&r,-6,0);
        if(pressed)OffsetRect(&r,1,1);
        UINT flags=DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX|DT_END_ELLIPSIS;
        UINT alignment=item.fmt&HDF_JUSTIFYMASK;
        flags|=alignment==HDF_RIGHT?DT_RIGHT:alignment==HDF_CENTER?DT_CENTER:DT_LEFT;
        if(item.fmt&HDF_RTLREADING)flags|=DT_RTLREADING;
        SetBkMode(dc,TRANSPARENT);
        SetTextColor(dc,p->text);
        DrawTextW(dc,text,-1,&r,flags);
    }
    return RestoreDC(dc,saved) && painted;
}
