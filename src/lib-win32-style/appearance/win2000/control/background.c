/* Win2000 GetControlBrush / PaintRect query and parent brush alignment. */
#include "background.h"
HBRUSH win2k_control_brush(HWND window,style_instance *instance,HDC dc,const win2k_palette *p)
{
    return win2k_control_color_brush(window,GetParent(window),instance,dc,WM_CTLCOLORSTATIC,p,p->face);
}
HBRUSH win2k_control_color_brush(HWND window,HWND parent,style_instance *instance,HDC dc,UINT message,
    const win2k_palette *p,COLORREF background)
{
    if(!parent)return NULL;
    HWND relation=GetParent(window);
    SetBkMode(dc,OPAQUE);SetTextColor(dc,p->text);SetBkColor(dc,background);
    HBRUSH brush=(HBRUSH)SendMessageW(parent,message,(WPARAM)dc,(LPARAM)window);
    if(instance->detached || !IsWindow(window) || GetParent(window)!=relation || !brush)return NULL;
    POINT origin={0,0};MapWindowPoints(parent,window,&origin,1);
    if(!LPtoDP(dc,&origin,1)||!SetBrushOrgEx(dc,origin.x,origin.y,NULL))return NULL;
    if(brush==GetSysColorBrush(COLOR_BTNFACE) || brush==GetSysColorBrush(COLOR_WINDOW)){
        COLORREF color=brush==GetSysColorBrush(COLOR_BTNFACE)?p->face:p->window;
        if(p->text!=win2k_standard_palette.text && GetTextColor(dc)==GetSysColor(COLOR_WINDOWTEXT))
            SetTextColor(dc,p->text);
        SetBkColor(dc,color);SetDCBrushColor(dc,color);brush=(HBRUSH)GetStockObject(DC_BRUSH);
    }
    return brush;
}
