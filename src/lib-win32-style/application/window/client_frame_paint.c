#include "client_frame_paint.h"

void style_client_frame_caption(HDC dc,const style_full_client_layout *layout,const style_window_palette *palette,
    const style_client_frame_paint_state *snapshot,PCWSTR title,BOOL child,BOOL zoomed)
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
