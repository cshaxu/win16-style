#include "combo_paint.h"
#include "basic_paint.h"
#include "../common/control_protocol.h"
#include <stdlib.h>

void win16_control_draw_combo(HWND w,const BOOL *detached,const BOOL *common_dialog_combo,const win16_style_palette *palette,HFONT *font_slot,HDC dc,BOOL (*get_info)(HWND,PCOMBOBOXINFO),BOOL (*is_vb_combo)(HWND),BOOL (*is_vb_drive)(HWND))
{
    COMBOBOXINFO info={.cbSize=sizeof(info)};RECT client,button,text,pixel;HFONT font;
    WCHAR *label;int length,saved,left,top,u;BOOL selected,pressed,editable,simple;
    if(!dc || !get_info(w,&info))return;
    length=GetWindowTextLengthW(w);label=calloc((size_t)length+1,sizeof(WCHAR));if(!label)return;
    GetWindowTextW(w,label,length+1);font=*font_slot && (is_vb_combo(w) || style_control_protocol_is(w,L"COMBOBOX"))?
        *font_slot:(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    pressed=SendMessageW(w,CB_GETDROPPEDSTATE,0,0)!=0;
    simple=(GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_SIMPLE;
    editable=(GetWindowLongPtrW(w,GWL_STYLE)&3)!=CBS_DROPDOWNLIST;
    if(*detached){free(label);return;}
    saved=SaveDC(dc);if(!saved){free(label);return;}
    u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));GetClientRect(w,&client);
    /* Native child EDIT owns its caret, text, selection and IME. Do not paint
     * over that child while drawing the combo's surrounding chrome. */
    if(editable || is_vb_drive(w) || *common_dialog_combo)ExcludeClipRect(dc,info.rcItem.left,info.rcItem.top,info.rcItem.right,info.rcItem.bottom);
    if(simple){RECT list;
        if(GetWindowRect(info.hwndList,&list)){
            MapWindowPoints(NULL,w,(POINT *)&list,2);ExcludeClipRect(dc,list.left,list.top,list.right,list.bottom);}
        SetDCBrushColor(dc,(*palette).light);FillRect(dc,&client,(HBRUSH)GetStockObject(DC_BRUSH));
        text=info.rcItem;InflateRect(&text,u,u);style_paint_solid_frame(dc,text,(*palette).edge,u);
        RestoreDC(dc,saved);free(label);return;
    }
    SetDCBrushColor(dc,(*palette).light);FillRect(dc,&client,(HBRUSH)GetStockObject(DC_BRUSH));
    style_paint_solid_frame(dc,client,(*palette).edge,u);
    button=info.rcButton;SetDCBrushColor(dc,(*palette).face);FillRect(dc,&button,(HBRUSH)GetStockObject(DC_BRUSH));
    style_paint_solid_frame(dc,button,(*palette).edge,u);InflateRect(&button,-u,-u);
    pixel=button;pixel.bottom=pixel.top+u;SetDCBrushColor(dc,pressed?(*palette).shadow:(*palette).light);
    FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));pixel=button;pixel.right=pixel.left+u;
    FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc,pressed?(*palette).light:(*palette).shadow);pixel=button;pixel.top=pixel.bottom-u;
    FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));pixel=button;pixel.left=pixel.right-u;
    FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    left=(button.left+button.right-7*u)/2;top=(button.top+button.bottom-9*u)/2;
    if(pressed){left+=u;top+=u;}
    SetDCBrushColor(dc,IsWindowEnabled(w)?(*palette).edge:(*palette).shadow);
    win16_control_combo_arrow(dc,left,top,96*u);
    if(editable || is_vb_drive(w) || *common_dialog_combo){RestoreDC(dc,saved);free(label);return;}
    text=info.rcItem;selected=GetFocus()==w && !pressed;
    SetDCBrushColor(dc,selected?(*palette).active:(*palette).light);FillRect(dc,&text,(HBRUSH)GetStockObject(DC_BRUSH));
    SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,!IsWindowEnabled(w)?(*palette).shadow:selected?(*palette).active_text:(*palette).edge);
    text.left+=2;DrawTextW(dc,label,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX);
    if(selected)DrawFocusRect(dc,&info.rcItem);
    RestoreDC(dc,saved);free(label);
}


void win16_control_native_combo_button(HWND w,const BOOL *detached,HDC dc,const RECT *dirty,RECT r,BOOL pressed)
{
    RECT q;POINT origin;win16_style_palette palette;
    if(!dc || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
       !GetWindowOrgEx(dc,&origin) || origin.x || origin.y ||
       !GetViewportOrgEx(dc,&origin) || origin.x || origin.y)return;
    int dpi=(int)GetDpiForWindow(w);if(dpi<=0 || dpi>768)return;
    int u=max(1,MulDiv(1,dpi,96)),gw=MulDiv(7,dpi,96),gh=MulDiv(9,dpi,96);
    if(r.right-r.left<gw+2*u || r.bottom-r.top<gh+2*u || *detached || !IsWindow(w))return;
    int saved=SaveDC(dc);if(!saved)return;
    if(IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)==ERROR ||
       (dirty && IntersectClipRect(dc,dirty->left,dirty->top,dirty->right,dirty->bottom)==ERROR)){
        RestoreDC(dc,saved);return;
    }
    win16_style_default_palette(&palette);HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    SetDCBrushColor(dc,palette.face);FillRect(dc,&r,brush);SetDCBrushColor(dc,palette.edge);
    for(int i=0;i<u;++i){FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);}
    q=r;SetDCBrushColor(dc,pressed?palette.shadow:palette.light);q.bottom=q.top+u;FillRect(dc,&q,brush);
    q=r;q.right=q.left+u;FillRect(dc,&q,brush);
    q=r;SetDCBrushColor(dc,pressed?palette.light:palette.shadow);q.top=q.bottom-u;FillRect(dc,&q,brush);
    q=r;q.left=q.right-u;FillRect(dc,&q,brush);
    int left=(r.left+r.right-gw)/2+(pressed?u:0),top=(r.top+r.bottom-gh)/2+(pressed?u:0);
    SetDCBrushColor(dc,IsWindowEnabled(w)?palette.edge:palette.shadow);
    win16_control_combo_arrow(dc,left,top,dpi);
    RestoreDC(dc,saved);
}
