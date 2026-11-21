#include "lib-win32-style/application/window/frame.h"
#include "frame_lifecycle.h"
#include "lib-win32-style/application/lifecycle/install.h"
#include "frame_dispatch.h"
#include "lib-win32-style/appearance/msstyles/frame_font.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/window/dwm.h"
#include <dwmapi.h>

extern IMAGE_DOS_HEADER __ImageBase;

typedef struct frame_install {
    style_window *state;
    const win16_control_frame_lifecycle_services *services;
    RECT client;
    BOOL normal;
} frame_install;
static BOOL setup(HWND window,void *context)
{
    frame_install *install=context;style_window *state=install->state;
    const win16_control_frame_lifecycle_services *services=install->services;
    /* Range changes can add bars after attachment: keep one tracker ready. */
    if(!services->scrollbars_install(window,&state->palette,state,&state->instance.detached))return FALSE;
    /* Native USER retains the normal placement of initially iconic/zoomed windows. */
    if(!state->native.pending)services->activate_frame(window,state,&install->client,install->normal);
    return TRUE;
}
static void rollback(HWND window,void *context)
{
    frame_install *install=context;
    (void)install->services->remove(window);
    if(!install->state->instance.detached)style_instance_detach(&install->state->instance);
}

BOOL win16_control_frame_install(HWND w,const win16_style_palette *p,BOOL creating,const void *owner,const BOOL *cancelled,const win16_control_frame_lifecycle_services *v)
{
    style_window *s;
    DWORD style,ex;
    BOOL normal;
    RECT client;
    if(!v->local_window(w))return FALSE;
    if(v->state(w)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    style=(DWORD)GetWindowLongPtr(w,GWL_STYLE);ex=(DWORD)GetWindowLongPtr(w,GWL_EXSTYLE);
    if(ex&(WS_EX_CLIENTEDGE|WS_EX_TOOLWINDOW)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
    }
    if(ex&WS_EX_LAYERED){
        WCHAR name[32];COLORREF key;BYTE alpha;DWORD flags;
        if(!GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpW(name,L"ConsoleWindowClass") ||
           !GetLayeredWindowAttributes(w,&key,&alpha,&flags)){
            SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
        }
    }
    normal=!IsIconic(w)&&!IsZoomed(w);
    s=win16_control_frame_state_create();
    if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->menu.bar.selected=-1;s->instance.owner=owner;
    s->native.pending=creating && normal && !(style&WS_VISIBLE);
    {WCHAR name[32];s->dialog=GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"#32770");}
    if(p)s->palette=*p;else style_window_palette_default(&s->palette);
    classic_theme_mapping source={0};
    DWORD theme_error=classic_theme_attachment_layout((HMODULE)&__ImageBase,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,&source);
    if(theme_error){win16_control_frame_state_dispose(s);SetLastError(theme_error);return FALSE;}
    if(source.resources){
        const classic_theme *theme=&source.model;
        s->title_buttons=theme->title_buttons;
        s->caption_metrics=theme->caption_metrics;
        s->caption_presentation=theme->caption_presentation;
        s->border_presentation=theme->border_presentation;
        s->caption_gradient=theme->gradient;
        s->active_end=theme->active.end;
        s->inactive_end=theme->inactive.end;
        s->caption_font=theme->caption_font;
        s->menu_font=theme->menu_font;
        s->menu_metrics=theme->menu_metrics;
        (void)style_menu_art_read(source.resources,theme->menu_images,&s->menu_art);
        (void)classic_caption_art_read(source.resources,&theme->caption_images,&s->art);
        if(!p){
            s->menu_bar=theme->menu_bar;
            s->menu_popup=theme->menu_popup;
            classic_theme_window_palette(&s->palette,theme);
        }
        classic_theme_close(&source);
    }
    s->native.active=(ex&WS_EX_MDICHILD)?
        (HWND)SendMessageW(GetParent(w),WM_MDIGETACTIVE,0,0)==w:GetActiveWindow()==w;
    if(!v->refresh_font(w,s)){win16_control_frame_state_dispose(s);SetLastError(ERROR_INVALID_DATA);return FALSE;}
    {style_full_client_layout checked;
        if(!v->layout(w,s,&checked)){win16_control_frame_state_dispose(s);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}}
    style_client_screen(w,&client);
    style_native_frame_capture(&s->native,w,v->get_window_attribute,FALSE);
    frame_install install={s,v,client,normal};
    const style_instance_install_ops ops={&install,cancelled,setup,rollback};
    return style_instance_install(&s->instance,w,v->style_proc,owner,&ops);
}
BOOL win16_control_menu_owner_install(HWND w,const void *owner,const BOOL *cancelled,const win16_control_frame_lifecycle_services *v)
{
 if(!v->local_window(w))return FALSE;
 if(v->state(w)){SetLastError(ERROR_INVALID_STATE);return FALSE;}
 style_window *s=win16_control_frame_state_create();if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
 s->instance.owner=owner;s->menu_only=TRUE;s->fonts.dpi=GetDpiForWindow(w);if(!s->fonts.dpi)s->fonts.dpi=96;
 style_window_palette_default(&s->palette);
 classic_theme_mapping source={0};
 DWORD theme_error=classic_theme_attachment_layout((HMODULE)&__ImageBase,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,&source);
 if(theme_error){win16_control_frame_state_dispose(s);SetLastError(theme_error);return FALSE;}
 if(source.resources){
  classic_theme_window_palette(&s->palette,&source.model);
  s->menu_popup=source.model.menu_popup;s->menu_metrics=source.model.menu_metrics;
  s->menu_font=source.model.menu_font;
  (void)style_menu_art_read(source.resources,source.model.menu_images,&s->menu_art);
  classic_theme_close(&source);
 }
 s->fonts.menu=style_frame_menu_font_acquire(s->fonts.dpi,&s->menu_font);
 if(!s->fonts.menu || (cancelled&&*cancelled)){
  DWORD error=cancelled&&*cancelled?ERROR_CANCELLED:ERROR_INVALID_DATA;
  win16_control_frame_state_dispose(s);SetLastError(error);return FALSE;
 }
 const style_instance_install_ops ops={.cancelled=cancelled};
 return style_instance_install(&s->instance,w,v->style_proc,owner,&ops);
}
BOOL win16_control_frame_remove_owned(HWND w,const void *owner,const win16_control_frame_lifecycle_services *v)
{
    style_window *s=v->state(w);
    if(!owner || !s || s->instance.owner!=owner)return FALSE;
    return v->remove(w);
}

BOOL win16_control_frame_remove(HWND w,const win16_control_frame_lifecycle_services *v)
{
    style_window *s;RECT client;BOOL normal=FALSE,menu_only;
    if(!v->local_window(w))return FALSE;
    s=v->state(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    menu_only=s->menu_only;
    if(!menu_only){normal=!IsIconic(w)&&!IsZoomed(w);style_client_screen(w,&client);}
    style_instance_retain(&s->instance);
    if(!style_instance_detach(&s->instance)){style_instance_release(&s->instance);return FALSE;}
    if(!menu_only)style_uwp_repaint_cancel(&s->xaml);
    if(!menu_only)v->control_remove(w,s);
    style_menu_owner_stop(&s->menu);
    if(!menu_only){
        if(s->menu.bar.keyboard)v->leave_bar(w,s,TRUE);
        if(s->button.pressed && GetCapture()==w)ReleaseCapture();
        (void)style_native_frame_restore_policy(w,&s->native,v->set_window_attribute);
        style_native_frame_restore_geometry(w,&s->native,&client,normal);
    }
    style_instance_release(&s->instance);
    return TRUE;
}

BOOL win16_control_frame_set_palette(HWND w,const win16_style_palette *p,const win16_control_frame_lifecycle_services *v)
{
    style_window *s;
    if(!p){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!v->local_window(w))return FALSE;
    s=v->state(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_instance_retain(&s->instance);s->palette=*p;v->scrollbars_palette(w,p,s);v->paint(w,s,NULL);
    style_instance_release(&s->instance);
    return TRUE;
}
BOOL win16_control_frame_terminal(HWND w,const win16_control_frame_lifecycle_services *v)
{
    style_window *s;
    s=v->state(w);if(!s)return TRUE;
    style_instance_retain(&s->instance);
    if(!style_instance_detach(&s->instance)){
        style_instance_release(&s->instance);SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    style_uwp_repaint_cancel(&s->xaml);
    style_menu_owner_stop(&s->menu);
    style_instance_release(&s->instance);
    return TRUE;
}
