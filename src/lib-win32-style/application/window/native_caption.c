#include "native_caption.h"
#include <string.h>

style_native_caption_colors style_native_caption_palette(BOOL active)
{
    BOOL gradient=FALSE;
    (void)SystemParametersInfoW(SPI_GETGRADIENTCAPTIONS,0,&gradient,0);
    return (style_native_caption_colors){
        GetSysColor(active?COLOR_ACTIVECAPTION:COLOR_INACTIVECAPTION),
        GetSysColor(active?COLOR_GRADIENTACTIVECAPTION:COLOR_GRADIENTINACTIVECAPTION),
        GetSysColor(active?COLOR_CAPTIONTEXT:COLOR_INACTIVECAPTIONTEXT),gradient};
}

int style_native_caption_height(UINT dpi,BOOL tool)
{
    return GetSystemMetricsForDpi(tool?SM_CYSMCAPTION:SM_CYCAPTION,dpi);
}

int style_native_caption_measure_height(UINT dpi,BOOL tool,const classic_caption_metrics *metrics,int floor)
{
    int height=metrics?(tool?metrics->tool_height:metrics->height):0;
    int pixels=height>0?MulDiv(height,(int)dpi,96):style_native_caption_height(dpi,tool);
    return max(pixels,floor);
}

BOOL style_native_caption_buttons_with_system(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[STYLE_NATIVE_CAPTION_BUTTONS],const classic_button_group *group,
    const classic_caption_metrics *metrics,RECT bounds[STYLE_NATIVE_CAPTION_BUTTONS],RECT *system)
{
    if(!title)return FALSE;
    SIZE defined=metrics?(tool?metrics->tool_button:metrics->button):(SIZE){0};
    SIZE size={
        style_caption_button_width(metrics,dpi,tool),
        defined.cy>0?MulDiv(defined.cy,(int)dpi,96):
            min(title->bottom-title->top,GetSystemMetricsForDpi(tool?SM_CYSMSIZE:SM_CYSIZE,dpi))};
    unsigned mask=system?1u<<CLASSIC_BUTTON_SYSTEM:0;
    for(unsigned i=0;i<STYLE_NATIVE_CAPTION_BUTTONS;++i)
        if(present[i])mask|=1u<<(CLASSIC_BUTTON_MINIMIZE+i);
    RECT slots[CLASSIC_BUTTON_CLOSE+1];
    if(!style_caption_slots(group,title,dpi,size,mask,FALSE,slots))return FALSE;
    memcpy(bounds,slots+CLASSIC_BUTTON_MINIMIZE,sizeof(*bounds)*STYLE_NATIVE_CAPTION_BUTTONS);
    if(system)*system=slots[CLASSIC_BUTTON_SYSTEM];
    return TRUE;
}

unsigned style_native_caption_left_buttons(const classic_button_group *group)
{
    unsigned left=0;
    for(unsigned i=0;i<group->count;++i)
        if(group->slots[i].side==CLASSIC_BUTTON_LEFT)left|=1u<<group->slots[i].role;
    return left;
}

BOOL style_native_caption_system_side(const classic_button_group *group,BOOL *on_right)
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

style_frame_colors style_native_frame_colors(const style_frame_colors *defined)
{
    if(defined && defined->present)return *defined;
    return (style_frame_colors){GetSysColor(COLOR_3DFACE),GetSysColor(COLOR_3DHIGHLIGHT),
        GetSysColor(COLOR_3DSHADOW),GetSysColor(COLOR_3DDKSHADOW),TRUE};
}

void style_native_frame_border_paint(HDC dc,RECT bounds,int thickness,const style_border_presentation *border,
    const style_frame_colors *colors,COLORREF title,BOOL dialog)
{
    if(!border->fill)return;
    const style_frame_colors frame=style_native_frame_colors(colors);
    const style_border_palette palette={frame.face,frame.edge,frame.light,frame.shadow,title};
    style_frame_border_paint(dc,bounds,thickness,1,dialog,NULL,border,&palette);
}

void style_native_caption_render(HDC dc,const style_native_caption_layout *layout,PCWSTR title,HICON icon,
    HFONT font,UINT dpi,style_native_caption_colors colors,const style_frame_colors *frame,
    const style_caption_presentation *presentation,BOOL maximized,
    const UINT button_flags[STYLE_NATIVE_CAPTION_BUTTONS],const classic_caption_art *art)
{
    RECT band=layout->band;
    if(IsRectEmpty(&band))return;
    int saved=SaveDC(dc);
    if(!saved)return;
    const style_caption_presentation defaults={0};
    if(!presentation)presentation=&defaults;
    const style_frame_colors resolved=style_native_frame_colors(frame);
    style_caption_render_model model={
        .band=band,.text=band,.start=colors.left,.end=colors.right,.ink=colors.text,
        .button_colors={resolved.edge,resolved.light,resolved.shadow,resolved.face},
        .gradient=colors.gradient,.maximized=maximized,.sprites=layout->sprites,.dpi=dpi,.active=layout->active,.unit=max(1,MulDiv(1,(int)dpi,96))
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
    for(unsigned i=0;i<STYLE_NATIVE_CAPTION_BUTTONS;++i){
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
