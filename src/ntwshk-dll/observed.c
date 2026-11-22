#include "observed.h"
#include "lib-win32-style/application/window/observed_frame.h"
#include "lib-win32-style/appearance/msstyles/observed_control_style.h"
#include "lib-win32-style/application/lifecycle/global_window.h"
#include "lib-win32-style/application/lifecycle/theme_state.h"
#include "lib-win32-style/application/window/window_identity.h"
#include <wchar.h>
#include "lib-win32-style/appearance/msstyles/native_caption_style.h"
#include "lib-win32-style/appearance/msstyles/native_scroll_style.h"

extern IMAGE_DOS_HEADER __ImageBase;

static DWORD load_frame(style_observed_frame_theme *frame,style_native_scroll *scroll)
{
    classic_theme_mapping source={0};
    DWORD error=classic_theme_attachment_layout((HMODULE)&__ImageBase,CLASSIC_WINDOW_PROVIDER_NONCLIENT,&source);
    if(error || !source.resources)return error;
    const classic_theme *theme=&source.model;
    *frame=(style_observed_frame_theme){
        .present=TRUE,.title_buttons=theme->title_buttons,.caption_metrics=theme->caption_metrics,
        .caption_presentation=theme->caption_presentation,.border_presentation=theme->border_presentation,
        .colors=theme->frame,.active=classic_theme_caption_palette(TRUE,theme),.inactive=classic_theme_caption_palette(FALSE,theme),
        .caption_font=theme->caption_font,.small_caption_font=theme->small_caption_font,
        .menu_font=theme->menu_font,.menu_bar=theme->menu_bar,.menu_text_flags=theme->menu_metrics.text_flags
    };
    classic_theme_native_menu(&frame->menu,&source);
    (void)classic_caption_art_read(source.resources,&theme->caption_images,&frame->art);
    classic_theme_native_scroll(scroll,theme);
    classic_theme_close(&source);
    return ERROR_SUCCESS;
}

static HFONT caption_font(const style_font_descriptor *font,UINT dpi,BOOL tool)
{return msstyles_font_create(font,dpi,tool?MSSTYLES_FONT_SMALL_CAPTION:MSSTYLES_FONT_CAPTION);}

static const style_observed_frame_provider frame_provider={
    L"Win2kStyle.Frame",L"Win2kStyle.RecalculateFrame",load_frame,caption_font
};
static void attach_frame(HWND window)
{if(window && !style_global_window_managed(window))style_observed_frame_attach(window,&frame_provider);}
static int control_owner;
static DWORD load_control_theme(style_observed_control_theme *theme)
{return classic_observed_control_load((HMODULE)&__ImageBase,theme);}

static BOOL control_local(HWND window)
{
    if(win32_style_window_current_thread(window))return TRUE;
    SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
}
BOOL ntwshk_observed_control_attached(HWND window)
{return control_local(window) && (style_observed_control_attached(window) || style_theme_owned(window,&control_owner));}
BOOL ntwshk_observed_control_attach(HWND window)
{
    if(!control_local(window))return FALSE;
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(ntwshk_observed_control_attached(window))return TRUE;
    BOOL native_requested;
    if(style_observed_control_attach(window,ntwshk_observed_native_theme,load_control_theme,&native_requested))return TRUE;
    if(native_requested || GetLastError()!=ERROR_NOT_SUPPORTED)return FALSE;
    if(style_theme_retained(window)){SetLastError(ERROR_BUSY);return FALSE;}
    WCHAR name[32];
    if(!GetClassNameW(window,name,ARRAYSIZE(name)))return FALSE;
    static const WCHAR *const classes[]={L"Button",L"Edit",L"ComboBox",L"ListBox",L"ScrollBar"};
    for(unsigned i=0;i<ARRAYSIZE(classes);++i){
        if(!_wcsicmp(name,classes[i])){
            if(!style_theme_capture(window))return FALSE;
            if(style_theme_suppress(window,&control_owner,ntwshk_observed_native_theme))return TRUE;
            (void)style_theme_terminal(window,&control_owner);return FALSE;
        }
    }
    SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
}
BOOL ntwshk_observed_control_remove(HWND window)
{
    if(!control_local(window))return FALSE;
    if(style_observed_control_attached(window))return style_observed_control_remove(window);
    if(!ntwshk_observed_control_attached(window)){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_theme_remove(window,&control_owner,ntwshk_observed_native_theme);return TRUE;
}

static BOOL is_child(HWND window){return (GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)!=0;}
BOOL __cdecl ntwshk_observed_window_attach(HWND window)
{
#ifdef NTWSHK_UNIFIED
    if(!win32_style_window_current_thread(window)){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    if(!classic_theme_require_layout(CLASSIC_WINDOW_PROVIDER_NONCLIENT))return FALSE;
#endif
    return is_child(window)?ntwshk_observed_control_attach(window):style_observed_frame_attach(window,&frame_provider);
}
BOOL __cdecl ntwshk_observed_window_attached(HWND window){return ntwshk_observed_control_attached(window) || (!is_child(window) && style_observed_frame_attached(window));}
BOOL __cdecl ntwshk_observed_window_remove(HWND window){return is_child(window) || ntwshk_observed_control_attached(window)?ntwshk_observed_control_remove(window):style_observed_frame_remove(window);}

void ntwshk_observed_window_created(HWND window,DWORD style)
{
    if(style&WS_CHILD)(void)ntwshk_observed_control_attach(window);
    else if(window)attach_frame(window);
}

static BOOL CALLBACK attach_dialog_child(HWND child,LPARAM context)
{(void)context;(void)ntwshk_observed_control_attach(child);return TRUE;}
void ntwshk_observed_dialog_initialize(HWND dialog)
{attach_frame(dialog);if(IsWindow(dialog))EnumChildWindows(dialog,attach_dialog_child,0);}
