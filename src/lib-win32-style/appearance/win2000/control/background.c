/* Win2000 GetControlBrush / PaintRect query and parent brush alignment. */
#include "background.h"
HBRUSH win2k_control_brush(HWND window,style_instance *instance,HDC dc,const win2k_palette *p)
{
    HWND parent=GetParent(window);if(!parent)return NULL;
    SetBkMode(dc,OPAQUE);SetTextColor(dc,p->text);SetBkColor(dc,p->face);
    HBRUSH brush=(HBRUSH)SendMessageW(parent,WM_CTLCOLORSTATIC,(WPARAM)dc,(LPARAM)window);
    if(instance->detached || !IsWindow(window) || GetParent(window)!=parent || !brush)return NULL;
    POINT origin={0,0};MapWindowPoints(parent,window,&origin,1);
    if(!LPtoDP(dc,&origin,1)||!SetBrushOrgEx(dc,origin.x,origin.y,NULL))return NULL;
    if(brush==GetSysColorBrush(COLOR_BTNFACE)){
        SetBkColor(dc,p->face);SetDCBrushColor(dc,p->face);brush=(HBRUSH)GetStockObject(DC_BRUSH);
    }
    return brush;
}
