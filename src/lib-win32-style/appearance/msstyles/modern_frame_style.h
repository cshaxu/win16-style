#ifndef STYLE_MSSTYLES_MODERN_FRAME_STYLE_H
#define STYLE_MSSTYLES_MODERN_FRAME_STYLE_H
#include "native_caption_style.h"
#include "frame_font.h"
#include "lib-win32-style/application/adapters/uwp/client_frame.h"
#include "lib-win32-style/application/adapters/uwp/observed_frame.h"

static inline HFONT classic_modern_caption_font(const style_font_descriptor *font,UINT dpi)
{return msstyles_font_create(font,dpi,MSSTYLES_FONT_CAPTION);}

static inline BOOL classic_modern_client_frame(HWND owner,HWND title,const style_uwp_frame_action *actions,
    unsigned count,const classic_theme_mapping *source)
{
    style_uwp_client_theme theme={.refresh_fonts=style_frame_fonts_refresh};
    style_window_palette_default(&theme.palette);
    if(source){
        const classic_theme *m=&source->model;
        theme.title_buttons=m->title_buttons;theme.caption_metrics=m->caption_metrics;
        theme.caption_presentation=m->caption_presentation;theme.border_presentation=m->border_presentation;
        theme.caption_font=m->caption_font;theme.menu_font=m->menu_font;
        theme.menu_bar=m->menu_bar;theme.menu_popup=m->menu_popup;theme.menu_metrics=m->menu_metrics;
        theme.active_end=m->active.end;theme.inactive_end=m->inactive.end;theme.gradient=m->gradient;
        classic_theme_window_palette(&theme.palette,m);
        (void)style_menu_art_read(source->resources,m->menu_images,&theme.menu_art);
        (void)classic_caption_art_read(source->resources,&m->caption_images,&theme.art);
    }
    return style_uwp_client_frame_apply(owner,title,actions,count,&theme);
}

static inline void classic_modern_observed_theme(style_uwp_observed_theme *theme,const classic_theme_mapping *source)
{
    const classic_theme *m=&source->model;
    *theme=(style_uwp_observed_theme){
        .present=TRUE,.buttons=m->title_buttons,.metrics=m->caption_metrics,.caption=m->caption_presentation,
        .border=m->border_presentation,.colors=m->frame,.active=classic_theme_caption_palette(TRUE,m),
        .inactive=classic_theme_caption_palette(FALSE,m),.font=m->caption_font,.create_font=classic_modern_caption_font
    };
    classic_theme_native_menu(&theme->menu,source);
    if(source->resources)(void)classic_caption_art_read(source->resources,&m->caption_images,&theme->art);
}
static inline BOOL classic_modern_client_current(HMODULE module,HWND owner,HWND title,
    const style_uwp_frame_action *actions,unsigned count)
{
    classic_theme_mapping source={0};
    DWORD error=classic_theme_attachment_layout(module,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,&source);
    if(error){SetLastError(error);return FALSE;}
    BOOL result=classic_modern_client_frame(owner,title,actions,count,source.resources?&source:NULL);
    error=GetLastError();
    classic_theme_close(&source);
    SetLastError(error);
    return result;
}

static inline BOOL classic_modern_frame_apply(HWND owner,HWND title,const style_uwp_frame_action *actions,
    unsigned count,const classic_theme_mapping *source)
{
    if(!title){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!source || !source->resources){SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);return FALSE;}
    switch(source->model.dialog_policy.window_provider){
    case CLASSIC_WINDOW_PROVIDER_FULL_CLIENT:
        return classic_modern_client_frame(owner,title,actions,count,source);
    case CLASSIC_WINDOW_PROVIDER_NONCLIENT:{
        style_uwp_observed_theme theme;
        classic_modern_observed_theme(&theme,source);
        return style_uwp_observed_frame_apply(owner,title,actions,count,&theme);
    }
    default:
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }
}
#endif
