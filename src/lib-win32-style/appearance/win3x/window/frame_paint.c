#include "frame_paint.h"
#include "../menu/menu_bar.h"

void win16_control_fill(HDC dc,RECT r,COLORREF c)
{
    SetDCBrushColor(dc,c);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
}

void win16_control_outline(HDC dc,RECT r,int u,COLORREF c)
{
    RECT q=r;SetDCBrushColor(dc,c);HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    q.bottom=q.top+u;FillRect(dc,&q,brush);
    q=r;q.top=q.bottom-u;FillRect(dc,&q,brush);
    q=r;q.right=q.left+u;FillRect(dc,&q,brush);
    q=r;q.left=q.right-u;FillRect(dc,&q,brush);
}

static void paint_button(HDC dc,RECT r,enum classic_button_role role,UINT flags,BOOL zoom,BOOL child,int u,
    const win16_style_palette *p,const classic_caption_art *art,const style_caption_presentation *presentation)
{
    const COLORREF palette[]={p->edge,p->light,p->shadow,p->face};
    const style_caption_raster raster=style_caption_button_raster(art,role,zoom,child,flags);
    style_caption_button_paint(dc,r,role,zoom,flags,u,presentation,&raster,palette);
}

void win16_style_merged_control(HDC dc,RECT bounds,unsigned role,UINT flags,int unit,
    const win16_style_palette *palette,const classic_caption_art *art,
    const style_caption_presentation *presentation)
{
    BOOL system=role==CLASSIC_MDI_SYSTEM;
    enum classic_button_role button=system?CLASSIC_BUTTON_SYSTEM:role==CLASSIC_MDI_CLOSE?CLASSIC_BUTTON_CLOSE:
        role==CLASSIC_MDI_MINIMIZE?CLASSIC_BUTTON_MINIMIZE:CLASSIC_BUTTON_MAXIMIZE;
    paint_button(dc,bounds,button,flags,role==CLASSIC_MDI_RESTORE,system,unit,palette,art,presentation);
}


void win16_control_frame_caption(HDC dc,const style_full_client_layout *layout,const win16_style_palette *palette,
    const win16_frame_paint_state *snapshot,PCWSTR title,BOOL child,BOOL zoomed)
{
    BOOL gradient=snapshot->gradient && !style_window_high_contrast();
    COLORREF start=snapshot->active?palette->active:palette->inactive;
    style_caption_render_model model={
        .band=layout->caption,.text=layout->title,.start=start,.end=gradient?snapshot->end:start,
        .ink=snapshot->active?palette->active_text:palette->inactive_text,
        .button_colors={palette->edge,palette->light,palette->shadow,palette->face},
        .gradient=gradient,.maximized=zoomed,.child=child,.unit=layout->unit
    };
    if(snapshot->icon){
        UINT dpi=snapshot->fonts?snapshot->fonts->dpi:96;
        model.icon=snapshot->icon;
        model.icon_bounds=layout->icon;
        model.icon_size=(SIZE){GetSystemMetricsForDpi(SM_CXSMICON,dpi),GetSystemMetricsForDpi(SM_CYSMICON,dpi)};
    }
    enum classic_button_role pressed=snapshot->button.hot?style_caption_role(snapshot->button.pressed):0;
    if(pressed)model.flags[pressed]=DFCS_PUSHED;
    if(snapshot->system_menu || (snapshot->bar_keyboard && snapshot->bar_selected==-2))
        model.flags[CLASSIC_BUTTON_SYSTEM]=DFCS_PUSHED;
    if(snapshot->close_disabled)model.flags[CLASSIC_BUTTON_CLOSE]=DFCS_INACTIVE;
    for(enum classic_button_role role=CLASSIC_BUTTON_SYSTEM;role<=CLASSIC_BUTTON_CLOSE;++role)
        model.buttons[role]=layout->buttons[role];
    style_caption_render(dc,title,snapshot->fonts?snapshot->fonts->title:NULL,&model,&snapshot->presentation,snapshot->art);
}
