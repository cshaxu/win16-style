#include "caption.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include <string.h>

win2k_caption_colors win2k_caption_palette(BOOL active)
{
    return active
        ?(win2k_caption_colors){RGB(0,0,128),RGB(166,202,240),RGB(255,255,255),TRUE}
        :(win2k_caption_colors){RGB(128,128,128),RGB(181,181,181),RGB(192,192,192),TRUE};
}

win2k_caption_colors win2k_caption_theme_palette(BOOL active,const classic_theme *theme)
{
    if(!theme)return win2k_caption_palette(active);
    classic_caption_colors colors=active?theme->active:theme->inactive;
    return (win2k_caption_colors){colors.start,colors.end,colors.text,theme->gradient};
}

HFONT win2k_caption_font(UINT dpi,BOOL tool,const msstyles_font *defined)
{
    if(defined && defined->points){
        LOGFONTW logical=msstyles_font_at_dpi(defined,dpi);
        return CreateFontIndirectW(&logical);
    }
    int height=MulDiv(8,(int)dpi,tool?96:72);
    return CreateFontW(-height,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
}

int win2k_caption_height(UINT dpi,BOOL tool)
{
    return style_frame_caption_height(tool?16:19,dpi,0);
}

int win2k_caption_theme_height(UINT dpi,BOOL tool,const classic_theme *theme)
{
    int height=theme?(tool?theme->caption_metrics.tool_height:theme->caption_metrics.height):0;
    return style_frame_caption_height(height>0?height:tool?16:19,dpi,
        theme?theme->border_presentation.geometry.caption_floor:0);
}

static SIZE button_size(BOOL tool)
{
    return tool?(SIZE){15,15}:(SIZE){16,14};
}

BOOL win2k_caption_buttons(const RECT *title,UINT dpi,BOOL tool,
                          const BOOL present[WIN2K_CAPTION_PARTS],RECT bounds[WIN2K_CAPTION_PARTS])
{
    static const classic_button_group group=WIN2000_TITLE_BUTTON_GROUP;
    return win2k_caption_buttons_with_group(title,dpi,tool,present,&group,NULL,bounds);
}

BOOL win2k_caption_buttons_with_group(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[WIN2K_CAPTION_PARTS],const classic_button_group *group,
    const classic_caption_metrics *metrics,RECT bounds[WIN2K_CAPTION_PARTS])
{
    SIZE size=button_size(tool);
    if(metrics){
        SIZE defined=tool?metrics->tool_button:metrics->button;
        if(defined.cx>0 && defined.cy>0)size=defined;
    }
    size=(SIZE){MulDiv(size.cx,(int)dpi,96),MulDiv(size.cy,(int)dpi,96)};
    unsigned mask=0;
    for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i)
        if(present[i])mask|=1u<<(CLASSIC_BUTTON_MINIMIZE+i);
    RECT slots[CLASSIC_BUTTON_CLOSE+1];
    if(!style_caption_slots(group,title,dpi,size,mask,FALSE,slots))return FALSE;
    memcpy(bounds,slots+CLASSIC_BUTTON_MINIMIZE,sizeof(*bounds)*WIN2K_CAPTION_PARTS);
    return TRUE;
}

BOOL win2k_caption_paint(HDC dc,const RECT *bounds,PCWSTR title,HFONT font,
                         const win2k_caption_colors *colors)
{
    if(!dc || !bounds || !title || !font || !colors ||
       bounds->right<=bounds->left || bounds->bottom<=bounds->top)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL ok=style_caption_fill(dc,*bounds,colors->left,colors->right,colors->gradient) &&
        style_caption_title(dc,*bounds,title,font,colors->text,STYLE_CAPTION_TEXT_LEFT_ELLIPSIS);
    if(!RestoreDC(dc,saved))ok=FALSE;
    return ok;
}

static const classic_frame_colors default_frame={
    RGB(212,208,200),RGB(255,255,255),RGB(128,128,128),RGB(0,0,0),TRUE
};

void win2k_window_border_paint(HDC dc,int width,int height,int thickness,const classic_theme *theme,BOOL active,BOOL dialog)
{
    static const style_border_presentation fallback=WIN2000_FRAME_BORDER;
    const classic_frame_colors *frame=theme && theme->frame.present?&theme->frame:&default_frame;
    const style_border_presentation *presentation=theme && theme->border_presentation.fill?
        &theme->border_presentation:&fallback;
    const style_border_palette palette={frame->face,frame->edge,frame->light,frame->shadow,
        win2k_caption_theme_palette(active,theme).left};
    style_frame_border_paint(dc,(RECT){0,0,width,height},thickness,1,dialog,NULL,presentation,&palette);
}

static void paint_icon(HDC dc,const RECT *icon,HICON image,UINT dpi)
{
    int width=min(icon->right-icon->left,GetSystemMetricsForDpi(SM_CXSMICON,dpi));
    int height=min(icon->bottom-icon->top,GetSystemMetricsForDpi(SM_CYSMICON,dpi));
    if(image && width>0 && height>0)DrawIconEx(dc,icon->left+(icon->right-icon->left-width)/2,
        icon->top+(icon->bottom-icon->top-height)/2,image,width,height,0,NULL,DI_NORMAL);
}

void win2k_caption_render(HDC dc,const win2k_caption_layout *layout,PCWSTR title,HICON icon,
                         HFONT font,UINT dpi,BOOL active,BOOL maximized,
                         const UINT button_flags[WIN2K_CAPTION_PARTS])
{
    win2k_caption_render_with_colors(dc,layout,title,icon,font,dpi,win2k_caption_palette(active),
        NULL,NULL,maximized,button_flags,NULL);
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
    if(!presentation || !presentation->text_mode){
        static const style_caption_presentation fallback={.text_mode=STYLE_CAPTION_TEXT_LEFT_ELLIPSIS,
            .separator=STYLE_CAPTION_SEPARATOR_INSIDE};
        presentation=&fallback;
    }
    int fill_right=band.right;
    for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i){
        RECT r=layout->buttons[i];
        if(!IsRectEmpty(&r) && r.left>=band.left)fill_right=min(fill_right,r.left);
    }
    if(!frame || !frame->present)frame=&default_frame;
    const COLORREF button_colors[]={frame->edge,frame->light,frame->shadow,frame->face};
    const int unit=max(1,MulDiv(1,(int)dpi,96));
    RECT text=style_caption_band(dc,band,fill_right,colors.left,colors.right,colors.gradient,
        presentation->separator,unit,frame->face);
    text.left=max(text.left,layout->icon.right)+2;
    style_caption_title(dc,text,title,font,colors.text,presentation->text_mode);
    for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i){
        style_caption_raster raster={0};
        if(art && art->available && i<=WIN2K_MAXIMIZE && !(button_flags[i]&DFCS_INACTIVE)){
            unsigned image=i==WIN2K_MINIMIZE?CLASSIC_IMAGE_MINIMIZE:
                maximized?CLASSIC_IMAGE_RESTORE:CLASSIC_IMAGE_MAXIMIZE;
            raster=(style_caption_raster){.roles=art->roles[image][(button_flags[i]&DFCS_PUSHED)!=0],
                .width=CLASSIC_CAPTION_ART_WIDTH,.height=CLASSIC_CAPTION_ART_HEIGHT,.stride=CLASSIC_CAPTION_ART_WIDTH};
        }
        style_caption_button_paint(dc,layout->buttons[i],(enum classic_button_role)(CLASSIC_BUTTON_MINIMIZE+i),
            maximized,button_flags[i],unit,presentation,&raster,button_colors);
    }
    if(icon && !IsRectEmpty(&layout->icon))paint_icon(dc,&layout->icon,icon,dpi);
    RestoreDC(dc,saved);
}
