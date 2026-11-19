#include "list_paint.h"
#include "basic_paint.h"
#include "control_state.h"
#include "../../../application/graphics/paint_frame.h"
#include "lib-win32-style/application/control/list_snapshot.h"
#include "lib-win32-style/application/control/control_class.h"

typedef struct list_render {
    const win16_style_palette *palette;
    const style_control_state *state;
} list_render;
static HFONT list_font(HWND window,void *context)
{
    const list_render *render=context;
    return render->state->font && style_control_vb_list(window)?render->state->font:(HFONT)SendMessageW(window,WM_GETFONT,0,0);
}
static void list_begin(HWND window,void *context,HDC dc,const style_control_list_snapshot *snapshot)
{
    (void)window;const list_render *render=context;
    SetDCBrushColor(dc,render->palette->light);FillRect(dc,&snapshot->client,(HBRUSH)GetStockObject(DC_BRUSH));
    SelectObject(dc,snapshot->font?snapshot->font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
}
static void list_item(HWND window,void *context,HDC dc,const style_control_list_snapshot *snapshot,const style_control_list_item *item)
{
    const list_render *render=context;const win16_style_palette *palette=render->palette;
    SetDCBrushColor(dc,item->selected>0?palette->active:palette->light);FillRect(dc,&item->row,(HBRUSH)GetStockObject(DC_BRUSH));
    SetTextColor(dc,!IsWindowEnabled(window)?palette->shadow:item->selected>0?palette->active_text:palette->edge);
    /* Original LBPrintCallback starts multicolumn text at0, ordinary text at2. */
    RECT label=item->row;if(!(snapshot->style&LBS_MULTICOLUMN))label.left+=2;
    if(snapshot->style&LBS_USETABSTOPS)
        TabbedTextOutW(dc,label.left,label.top,item->text,item->length,render->state->tab_count,render->state->tab_positions,item->row.left);
    else DrawTextW(dc,item->text,-1,&label,DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
    if(GetFocus()==window && snapshot->caret==item->index)DrawFocusRect(dc,&item->row);
}
void win16_control_draw_list(HWND window,control_style *style,HDC dc)
{
    win16_style_palette palette=style->palette;
    if(style->list_colors.present){
        palette.light=style->list_colors.background;
        palette.active=style->list_colors.selection;
        palette.active_text=style->list_colors.selection_text;
    }
    if(style->label_colors.present){
        palette.edge=style->label_colors.normal;
        palette.shadow=style->label_colors.disabled;
    }
    list_render render={&palette,&style->state};
    const style_control_list_painter painter={&render,list_font,list_begin,list_item};
    style_control_list_paint(window,&style->instance,dc,&painter);
}

void win16_control_edit_border(HWND w,const control_style *style,HDC target)
{
    HDC dc;RECT bounds;int saved;
    if(!style->edit_border)return;
    const classic_edit_border *border=&style->edit_colors[classic_edit_state_for(w)];
    COLORREF edge=border->present?border->line:style->palette.edge;
    dc=target?target:GetDC(w);if(!dc)return;
    saved=SaveDC(dc);if(saved){
        GetClientRect(w,&bounds);style_paint_solid_frame(dc,bounds,edge,max(1,MulDiv(1,(int)GetDpiForWindow(w),96)));
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
