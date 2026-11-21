#include "caption.h"
#include <string.h>

win2k_caption_colors win2k_caption_palette(BOOL active)
{
    BOOL gradient=FALSE;
    (void)SystemParametersInfoW(SPI_GETGRADIENTCAPTIONS,0,&gradient,0);
    return (win2k_caption_colors){
        GetSysColor(active?COLOR_ACTIVECAPTION:COLOR_INACTIVECAPTION),
        GetSysColor(active?COLOR_GRADIENTACTIVECAPTION:COLOR_GRADIENTINACTIVECAPTION),
        GetSysColor(active?COLOR_CAPTIONTEXT:COLOR_INACTIVECAPTIONTEXT),gradient};
}

win2k_caption_colors win2k_caption_theme_palette(BOOL active,const classic_theme *theme)
{
    if(!theme)return win2k_caption_palette(active);
    classic_caption_colors colors=active?theme->active:theme->inactive;
    return (win2k_caption_colors){colors.start,colors.end,colors.text,theme->gradient};
}

int win2k_caption_height(UINT dpi,BOOL tool)
{
    return GetSystemMetricsForDpi(tool?SM_CYSMCAPTION:SM_CYCAPTION,dpi);
}

int win2k_caption_theme_height(UINT dpi,BOOL tool,const classic_theme *theme)
{
    int height=theme?(tool?theme->caption_metrics.tool_height:theme->caption_metrics.height):0;
    int pixels=height>0?MulDiv(height,(int)dpi,96):win2k_caption_height(dpi,tool);
    return theme?max(pixels,theme->border_presentation.geometry.caption_floor):pixels;
}

BOOL win2k_caption_buttons_with_system(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[WIN2K_CAPTION_PARTS],const classic_button_group *group,
    const classic_caption_metrics *metrics,RECT bounds[WIN2K_CAPTION_PARTS],RECT *system)
{
    if(!title)return FALSE;
    SIZE defined=metrics?(tool?metrics->tool_button:metrics->button):(SIZE){0};
    SIZE size={
        style_caption_button_width(metrics,dpi,tool),
        defined.cy>0?MulDiv(defined.cy,(int)dpi,96):
            min(title->bottom-title->top,GetSystemMetricsForDpi(tool?SM_CYSMSIZE:SM_CYSIZE,dpi))};
    unsigned mask=system?1u<<CLASSIC_BUTTON_SYSTEM:0;
    for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i)
        if(present[i])mask|=1u<<(CLASSIC_BUTTON_MINIMIZE+i);
    RECT slots[CLASSIC_BUTTON_CLOSE+1];
    if(!style_caption_slots(group,title,dpi,size,mask,FALSE,slots))return FALSE;
    memcpy(bounds,slots+CLASSIC_BUTTON_MINIMIZE,sizeof(*bounds)*WIN2K_CAPTION_PARTS);
    if(system)*system=slots[CLASSIC_BUTTON_SYSTEM];
    return TRUE;
}

unsigned win2k_caption_left_buttons(const classic_button_group *group)
{
    unsigned left=0;
    for(unsigned i=0;i<group->count;++i)
        if(group->slots[i].side==CLASSIC_BUTTON_LEFT)left|=1u<<group->slots[i].role;
    return left;
}

BOOL win2k_caption_system_side(const classic_button_group *group,BOOL *on_right)
{
    for(unsigned i=0;i<group->count;++i){
        classic_button_slot slot=group->slots[i];
        if(slot.role==CLASSIC_BUTTON_SYSTEM){
            *on_right=slot.side==CLASSIC_BUTTON_RIGHT;
            return TRUE;
        }
    }
    return FALSE;
}

classic_frame_colors win2k_window_frame_colors(const classic_frame_colors *defined)
{
    if(defined && defined->present)return *defined;
    return (classic_frame_colors){GetSysColor(COLOR_3DFACE),GetSysColor(COLOR_3DHIGHLIGHT),
        GetSysColor(COLOR_3DSHADOW),GetSysColor(COLOR_3DDKSHADOW),TRUE};
}

void win2k_window_border_paint(HDC dc,int width,int height,int thickness,const classic_theme *theme,BOOL active,BOOL dialog)
{
    if(!theme || !theme->border_presentation.fill)return;
    const classic_frame_colors frame=win2k_window_frame_colors(&theme->frame);
    const style_border_palette palette={frame.face,frame.edge,frame.light,frame.shadow,
        win2k_caption_theme_palette(active,theme).left};
    style_frame_border_paint(dc,(RECT){0,0,width,height},thickness,1,dialog,NULL,&theme->border_presentation,&palette);
}

void win2k_caption_render_with_colors(HDC dc,const win2k_caption_layout *layout,PCWSTR title,HICON icon,
    HFONT font,UINT dpi,win2k_caption_colors colors,const classic_frame_colors *frame,
    const style_caption_presentation *presentation,BOOL maximized,
    const UINT button_flags[WIN2K_CAPTION_PARTS],const classic_caption_art *art)
{
    RECT band=layout->band;
    if(IsRectEmpty(&band))return;
    int saved=SaveDC(dc);
    if(!saved)return;
    const style_caption_presentation defaults={0};
    if(!presentation)presentation=&defaults;
    const classic_frame_colors resolved=win2k_window_frame_colors(frame);
    style_caption_render_model model={
        .band=band,.text=band,.start=colors.left,.end=colors.right,.ink=colors.text,
        .button_colors={resolved.edge,resolved.light,resolved.shadow,resolved.face},
        .gradient=colors.gradient,.maximized=maximized,.unit=max(1,MulDiv(1,(int)dpi,96))
    };
    model.text.left=max(model.text.left,layout->icon.right);
    if(icon && !IsRectEmpty(&layout->icon)){
        model.icon=icon;
        model.icon_bounds=layout->icon;
        model.icon_size=(SIZE){GetSystemMetricsForDpi(SM_CXSMICON,dpi),GetSystemMetricsForDpi(SM_CYSMICON,dpi)};
    }
    if(!IsRectEmpty(&layout->system)){
        if(layout->system_on_right)model.text.right=min(model.text.right,layout->system.left);
        else model.text.left=max(model.text.left,layout->system.right);
        model.buttons[CLASSIC_BUTTON_SYSTEM]=layout->system;
        model.flags[CLASSIC_BUTTON_SYSTEM]=layout->system_flags;
    }
    for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i){
        unsigned role=CLASSIC_BUTTON_MINIMIZE+i;
        RECT bounds=layout->buttons[i];
        model.buttons[role]=bounds;
        model.flags[role]=button_flags[i];
        if(IsRectEmpty(&bounds))continue;
        if(layout->left_buttons&(1u<<role))model.text.left=max(model.text.left,bounds.right);
        else model.text.right=min(model.text.right,bounds.left);
    }
    style_caption_render(dc,title,font,&model,presentation,art);
    RestoreDC(dc,saved);
}
