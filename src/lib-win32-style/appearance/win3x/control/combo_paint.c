#include "combo_paint.h"
#include "basic_paint.h"
#include "control_state.h"
#include "lib-win32-style/application/control/control_class.h"
#include "../../../application/graphics/paint_frame.h"
#include "../../../application/control/control_protocol.h"
#include "../../../application/graphics/dc_geometry.h"
#include "lib-win32-style/application/control/combo_snapshot.h"

typedef struct combo_reader_context {
    const style_control_state *state;
    BOOL (*is_vb_combo)(HWND);
} combo_reader_context;
static HFONT combo_font(HWND window,void *context)
{
    const combo_reader_context *reader=context;
    return reader->state->font && (reader->is_vb_combo(window) || style_control_protocol_is(window,L"COMBOBOX"))?
        reader->state->font:(HFONT)SendMessageW(window,WM_GETFONT,0,0);
}
void win16_control_draw_combo(HWND w,control_style *style,HDC dc,BOOL (*get_info)(HWND,PCOMBOBOXINFO),BOOL (*is_vb_combo)(HWND),BOOL (*is_vb_drive)(HWND))
{
    RECT client,button,text,pixel;int saved,left,top,u;BOOL selected;
    if(!dc)return;
    combo_reader_context context={&style->state,is_vb_combo};const style_control_combo_reader reader={&context,get_info,combo_font};
    style_control_combo_snapshot snapshot;if(!style_control_combo_snapshot_read(w,&style->instance,&reader,&snapshot))return;
    COMBOBOXINFO info=snapshot.info;WCHAR *label=snapshot.text;HFONT font=snapshot.font;
    BOOL pressed=snapshot.pressed,simple=snapshot.simple,editable=snapshot.editable;
    BOOL enabled=IsWindowEnabled(w),common_dialog_combo=style->common_dialog_combo;
    const win16_style_palette *palette=&style->palette;
    const classic_combo_colors *colors=&style->combo_colors;
    BOOL themed=colors->present && !common_dialog_combo && style_control_is_combo(w);
    const classic_list_colors *list=&style->list_colors;
    const classic_text_colors *ink=&style->label_colors;
    BOOL list_themed=themed && list->present,text_themed=themed && ink->present;
    COLORREF background=list_themed?list->background:palette->light;
    COLORREF border=themed?(enabled?colors->border:colors->disabled_border):palette->edge;
    COLORREF face=themed?colors->button[classic_combo_state_for(enabled,pressed)]:palette->face;
    saved=SaveDC(dc);if(!saved){style_control_combo_snapshot_clear(&snapshot);return;}
    u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));GetClientRect(w,&client);
    /* Native child EDIT owns its caret, text, selection and IME. Do not paint
     * over that child while drawing the combo's surrounding chrome. */
    if(editable || is_vb_drive(w) || common_dialog_combo)ExcludeClipRect(dc,info.rcItem.left,info.rcItem.top,info.rcItem.right,info.rcItem.bottom);
    if(simple){RECT list;
        if(GetWindowRect(info.hwndList,&list)){
            MapWindowPoints(NULL,w,(POINT *)&list,2);ExcludeClipRect(dc,list.left,list.top,list.right,list.bottom);}
        SetDCBrushColor(dc,background);FillRect(dc,&client,(HBRUSH)GetStockObject(DC_BRUSH));
        text=info.rcItem;InflateRect(&text,u,u);style_paint_solid_frame(dc,text,border,u);
        RestoreDC(dc,saved);style_control_combo_snapshot_clear(&snapshot);return;
    }
    SetDCBrushColor(dc,background);FillRect(dc,&client,(HBRUSH)GetStockObject(DC_BRUSH));
    style_paint_solid_frame(dc,client,border,u);
    button=info.rcButton;SetDCBrushColor(dc,face);FillRect(dc,&button,(HBRUSH)GetStockObject(DC_BRUSH));
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
    if(editable || is_vb_drive(w) || common_dialog_combo){RestoreDC(dc,saved);style_control_combo_snapshot_clear(&snapshot);return;}
    text=info.rcItem;selected=GetFocus()==w && !pressed;
    COLORREF selection=list_themed?list->selection:palette->active;
    COLORREF selected_text=list_themed?list->selection_text:palette->active_text;
    COLORREF normal_text=text_themed?ink->normal:palette->edge,disabled_text=text_themed?ink->disabled:palette->shadow;
    SetDCBrushColor(dc,selected?selection:background);FillRect(dc,&text,(HBRUSH)GetStockObject(DC_BRUSH));
    SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,!enabled?disabled_text:selected?selected_text:normal_text);
    text.left+=2;DrawTextW(dc,label,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX);
    if(selected)DrawFocusRect(dc,&info.rcItem);
    RestoreDC(dc,saved);style_control_combo_snapshot_clear(&snapshot);
}


void win16_control_native_combo_button(HWND w,const style_instance *instance,HDC dc,const RECT *dirty,const style_control_native_button *button,const win16_style_palette *theme)
{
    RECT r=button->bounds;BOOL pressed=button->pressed;
    RECT q;win16_style_palette palette;
    if(!dc || !style_dc_unmirrored_origin_pixels(dc))return;
    int dpi=(int)GetDpiForWindow(w);if(dpi<=0 || dpi>768)return;
    int u=max(1,MulDiv(1,dpi,96)),gw=MulDiv(7,dpi,96),gh=MulDiv(9,dpi,96);
    if(r.right-r.left<gw+2*u || r.bottom-r.top<gh+2*u || instance->detached || !IsWindow(w))return;
    int saved=SaveDC(dc);if(!saved)return;
    if(IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)==ERROR ||
       (dirty && IntersectClipRect(dc,dirty->left,dirty->top,dirty->right,dirty->bottom)==ERROR)){
        RestoreDC(dc,saved);return;
    }
    if(theme)palette=*theme;else win16_style_default_palette(&palette);
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
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
