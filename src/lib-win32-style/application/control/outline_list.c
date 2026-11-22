#include "outline_list.h"
#include "control_chrome.h"

void style_outline_edit_border(HWND w,HDC target,const style_window_palette *palette,const style_frame_colors *frame,
    const classic_edit_border *borders,style_line_profile profile)
{
    HDC dc;RECT bounds;int saved;
    const classic_edit_border *border=&borders[classic_edit_state_for(w)];
    COLORREF edge=border->present?border->line:palette->edge;
    dc=target?target:GetDC(w);if(!dc)return;
    saved=SaveDC(dc);if(saved){
        style_relief_palette colors=style_control_outline_palette(palette);
        if(frame->present){
            colors.shadow=frame->shadow;
            colors.highlight=frame->light;
        }
        if(GetClientRect(w,&bounds))style_paint_line_frame(dc,&bounds,BDR_SUNKENOUTER,0,
            max(1,MulDiv(1,(int)GetDpiForWindow(w),96)),edge,profile,&colors);
        RestoreDC(dc,saved);
    }
    if(!target)ReleaseDC(w,dc);
}

void style_outline_list_border(HWND w,HDC target,const style_window_palette *palette,const style_frame_colors *frame)
{
    RECT bounds,inside;HDC dc;int saved;BOOL edge=(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)!=0;
    if(!(GetWindowLongPtrW(w,GWL_STYLE)&WS_BORDER) && !edge)return;
    if(!GetWindowRect(w,&bounds))return;
    OffsetRect(&bounds,-bounds.left,-bounds.top);
    dc=target?target:GetWindowDC(w);if(!dc)return;
    saved=SaveDC(dc);
    if(saved){
        if(edge){
            inside=bounds;InflateRect(&inside,-1,-1);
            style_paint_solid_frame(dc,inside,frame->present?frame->light:palette->light,1);
        }
        style_paint_solid_frame(dc,bounds,frame->present?frame->edge:palette->edge,1);
        RestoreDC(dc,saved);}
    if(!target)ReleaseDC(w,dc);
}
