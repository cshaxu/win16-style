#include "list_paint.h"
#include "basic_paint.h"
#include "control_state.h"
#include "../../../application/graphics/paint_frame.h"
#include "lib-win32-style/application/control/list_snapshot.h"
#include "lib-win32-style/application/control/control_class.h"

void win16_control_draw_list(HWND window,control_style *style,HDC dc)
{
    const classic_list_colors *list=&style->list_colors;
    const classic_text_colors *label=&style->label_colors;
    const win16_style_palette *palette=&style->palette;
    const style_list_appearance appearance={
        .font_override=style->state.font && style_control_vb_list(window)?style->state.font:NULL,
        .background=list->present?list->background:palette->light,
        .selection=list->present?list->selection:palette->active,
        .text=label->present?label->normal:palette->edge,
        .selected_text=list->present?list->selection_text:palette->active_text,
        .disabled_text=label->present?label->disabled:palette->shadow,
        .tabs=style->state.tab_positions,.tab_count=style->state.tab_count,.use_tabs=TRUE
    };
    style_control_list_strings(window,&style->instance,dc,&appearance);
}

void win16_control_edit_border(HWND w,const control_style *style,HDC target)
{
    HDC dc;RECT bounds;int saved;
    if(!style->edit_border || !style->controls.edit)return;
    const classic_edit_border *border=&style->edit_colors[classic_edit_state_for(w)];
    COLORREF edge=border->present?border->line:style->palette.edge;
    dc=target?target:GetDC(w);if(!dc)return;
    saved=SaveDC(dc);if(saved){
        style_relief_palette colors=win16_control_push_palette(&style->palette);
        if(style->frame_colors.present){
            colors.shadow=style->frame_colors.shadow;
            colors.highlight=style->frame_colors.light;
        }
        if(GetClientRect(w,&bounds))style_paint_line_frame(dc,&bounds,BDR_SUNKENOUTER,0,
            max(1,MulDiv(1,(int)GetDpiForWindow(w),96)),edge,style->controls.edit,&colors);
        RestoreDC(dc,saved);
    }
    if(!target)ReleaseDC(w,dc);
}

void win16_control_list_border(HWND w,const control_style *style,HDC target)
{
    RECT bounds,inside;HDC dc;int saved;BOOL edge=(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)!=0;
    if(!(GetWindowLongPtrW(w,GWL_STYLE)&WS_BORDER) && !edge)return;
    if(!GetWindowRect(w,&bounds))return;
    OffsetRect(&bounds,-bounds.left,-bounds.top);
    dc=target?target:GetWindowDC(w);if(!dc)return;
    saved=SaveDC(dc);
    if(saved){
        const classic_frame_colors *frame=&style->frame_colors;
        if(edge){
            inside=bounds;InflateRect(&inside,-1,-1);
            style_paint_solid_frame(dc,inside,frame->present?frame->light:style->palette.light,1);
        }
        style_paint_solid_frame(dc,bounds,frame->present?frame->edge:style->palette.edge,1);
        RestoreDC(dc,saved);}
    if(!target)ReleaseDC(w,dc);
}
