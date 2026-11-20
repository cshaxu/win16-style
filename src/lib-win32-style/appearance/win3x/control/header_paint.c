#include "header_paint.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>
#include "lib-win32-style/application/control/header_snapshot.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

BOOL win16_style_header_supported(HWND w)
{return !(GetWindowLongPtrW(w,GWL_STYLE)&(HDS_FILTERBAR|HDS_CHECKBOXES|HDS_OVERFLOW|HDS_HIDDEN));}

static void fill(HDC dc,RECT r,COLORREF color)
{SetDCBrushColor(dc,color);FillRect(dc,&r,GetStockObject(DC_BRUSH));}
BOOL win16_style_header_paint(HWND w,HDC dc,const win16_style_palette *p,HFONT font,
    const style_instance *instance,const style_control_native_state *state,const classic_header_colors *colors,
    style_line_profile profile)
{
    BOOL painted=FALSE;int saved=0;style_header_snapshot snapshot;
    if(!dc || !win16_style_header_supported(w) || !style_header_snapshot_read(w,instance,state,HDF_STRING|HDF_JUSTIFYMASK|HDF_RTLREADING,&snapshot))return FALSE;
    RECT client=snapshot.client;int count=snapshot.count;style_header_item *items=snapshot.items;
    saved=SaveDC(dc);if(!saved)goto done;
    IntersectClipRect(dc,client.left,client.top,client.right,client.bottom);
    COLORREF normal=colors && colors->present?colors->normal:p->face;
    fill(dc,client,normal);SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,IsWindowEnabled(w)?p->edge:p->shadow);
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    RECT bottom=client;bottom.top=max(bottom.top,bottom.bottom-u);fill(dc,bottom,p->edge);
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    BOOL button=(style&HDS_BUTTONS)!=0,captured=button && GetCapture()==w;
    const style_relief_palette edge={.face=normal,.light=p->face,.highlight=p->light,.shadow=p->shadow,.dark=p->edge};
    if(!profile)profile=STYLE_LINE_FLAT;
    for(int i=0;i<count;++i){
        RECT r=items[i].rect,visible;if(!IntersectRect(&visible,&client,&r))continue;
        int inner=SaveDC(dc);if(!inner)goto done;
        IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
        BOOL pressed=captured && i==state->pressed;
        RECT frame=r;
        for(int n=0;n<u && !IsRectEmpty(&r);++n)InflateRect(&r,-1,-1);
        if(pressed){
            if(colors && colors->present && colors->pressed!=normal){
                RECT interior=r;interior.bottom=min(interior.bottom,bottom.top);
                fill(dc,interior,colors->pressed);
            }
            OffsetRect(&r,u,u);
        }
        if(!style_paint_header_edge(dc,frame,button,(style&WS_BORDER)!=0,pressed,u,profile,&edge)){
            RestoreDC(dc,inner);
            goto done;
        }
        InflateRect(&r,-3*u,0);
        UINT flags=DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX|DT_END_ELLIPSIS;
        int alignment=items[i].format&HDF_JUSTIFYMASK;
        flags|=alignment==HDF_RIGHT?DT_RIGHT:alignment==HDF_CENTER?DT_CENTER:DT_LEFT;
        if((items[i].format&HDF_RTLREADING) || (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_RTLREADING))flags|=DT_RTLREADING;
        DrawTextW(dc,items[i].text,-1,&r,flags);RestoreDC(dc,inner);
    }
    painted=style_control_native_current(w,instance,state,snapshot.epoch);
done:
    if(saved)RestoreDC(dc,saved);
    style_header_snapshot_clear(&snapshot);return painted;
}
