/* Project-authored experimental non-client presentation, not imported USER.
 * See T445 S1 provenance/design: original NTUSER server is a stopping boundary.
 */
#include "win16_style.h"
#include "lib-win32-style/application/window/full_client_layout.h"
#include "../lib-win32-style/application/window/client_frame_paint.h"
#include "../lib-win32-style/application/window/client_frame_renderer.h"
#include "../lib-win32-style/application/window/client_frame_state.h"
#include "../lib-win32-style/application/window/client_frame_lifecycle.h"
#include "lib-win32-style/application/menu/bar_controller.h"
#include "lib-win32-style/application/window/messages.h"
#include "../lib-win32-style/application/window/client_frame_dispatch.h"
#include "../lib-win32-style/application/window/client_frame_input.h"
#include "../lib-win32-style/application/window/client_frame_layout.h"
#include "lib-win32-style/appearance/msstyles/frame_font.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/window/frame.h"
#include "lib-win32-style/application/window/window_identity.h"
#include "lib-win32-style/application/menu/client_bar.h"
#include "lib-win32-style/application/menu/owned_rows.h"
#include "lib-win32-style/application/menu/client_popup.h"
#include "controls.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/lifecycle/dispatch.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <windowsx.h>
#include <string.h>

extern IMAGE_DOS_HEADER __ImageBase;

static LRESULT CALLBACK style_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK style_proc_body(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

/* -3 means outside the bar/system cell; -2 denotes the system cell. */
static style_client_frame *state(HWND w)
{
    return (style_client_frame *)style_instance_lookup(w,style_proc);
}

static BOOL local_window(HWND w)
{
    if(!win32_style_window_current_thread(w)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return TRUE;
}

static BOOL layout(HWND w,style_client_frame *s,style_full_client_layout *l)
{
    RECT r;
    return GetWindowRect(w,&r) && style_client_frame_measure_window(w,&s->instance,&s->fonts,&r,
        &s->title_buttons,&s->caption_metrics,&s->border_presentation.geometry,s->caption_presentation.icon_mode,&s->menu_metrics,l);
}

BOOL win16_style_hit_test(HWND w,LPARAM lp,LRESULT *result)
{
    style_client_frame *s;style_full_client_layout l;
    if(!result || !local_window(w))return FALSE;
    s=state(w);
    if(!s || !layout(w,s,&l))return FALSE;
    return style_client_frame_hit_test(w,&l,lp,result);
}

static BOOL paint_layout(HWND w,void *context,style_full_client_layout *result)
{return layout(w,(style_client_frame *)context,result);}
static void paint_colors(void *context,win16_style_palette *palette)
{style_window_palette_resolve(&((style_client_frame *)context)->palette,palette);}
COLORREF win16_style_dialog_edge(HWND window)
{
    style_client_frame *frame=state(window);
    win16_style_palette palette;
    if(frame && !frame->instance.detached)paint_colors(frame,&palette);
    else style_window_palette_default(&palette);
    return palette.edge;
}
static void paint_state(void *context,style_client_frame_paint_state *result)
{
    style_client_frame *s=context;
    *result=(style_client_frame_paint_state){&s->fonts,s->native.active,s->menu.system,
        s->menu.bar.keyboard,s->menu.bar.selected,s->button,&s->art,&s->menu_bar,
        s->caption_presentation,s->caption_gradient,s->native.active?s->active_end:s->inactive_end,
        s->border_presentation,&s->menu_metrics,FALSE,NULL};
}
static const style_client_frame_renderer_ops renderer={paint_layout,paint_state,paint_colors,win16_style_scrollbars_paint};
static void paint(HWND w,void *context,HDC target)
{style_client_frame *s=context;style_client_frame_paint(w,&s->instance,s,&renderer,target);}
static void paint_buffered_callback(HWND w,void *context,HDC dc)
{style_client_frame *s=context;style_client_frame_buffered(w,&s->instance,s,&renderer,dc);}
static void paint_menu_state(HWND w,style_client_frame *s)
{style_client_frame_menu_state(w,&s->instance,s,&renderer);}
static void paint_button_state(HWND w,style_client_frame *s,int hit)
{style_client_frame_button_state(w,&s->instance,s,&renderer,hit);}
static void button_paint_callback(HWND w,void *context,int hit){paint_button_state(w,(style_client_frame *)context,hit);}
static style_client_frame_button_view button_view(style_client_frame *s)
{
    const style_client_frame_button_view view={s,&s->button,&s->instance,paint_layout,button_paint_callback};
    return view;
}

static BOOL bar_read(const style_menu_bar_target *target,style_menu_bar_snapshot *bar)
{
    style_client_frame *s=target->context;style_full_client_layout l;RECT outer;
    if(!layout(target->window,s,&l) || !GetWindowRect(target->window,&outer))return FALSE;
    if(!style_menu_client_bar_read(target->window,GetMenu(target->window),s->fonts.menu,s->fonts.dpi,
        &l,(POINT){l.menu.left,l.menu.top},l.menu.right-l.menu.left,bar))return FALSE;
    for(UINT i=0;i<bar->count;++i)OffsetRect(&bar->rects[i],outer.left,outer.top);
    return TRUE;
}
static int bar_hit(const style_menu_bar_target *target,POINT point)
{
    style_client_frame *s=target->context;style_full_client_layout l;
    if(s->instance.detached || !layout(target->window,s,&l))return STYLE_MENU_NO_ITEM;
    return style_menu_client_hit_screen(target->window,&l,s->fonts.menu,s->fonts.dpi,point);
}
static void bar_paint(const style_menu_bar_target *target)
{paint_menu_state(target->window,target->context);}
static BOOL bar_target(HWND,style_menu_bar_target *);
static const style_menu_bar_ops bar_ops={bar_read,bar_hit,bar_paint,bar_target,STYLE_MENU_TRACK_COLUMNS};
static BOOL bar_target(HWND w,style_menu_bar_target *target)
{
    style_client_frame *s=state(w);if(!s)return FALSE;
    *target=(style_menu_bar_target){w,&s->instance,&s->menu,s,&bar_ops};return TRUE;
}
static style_menu_bar_target menu_target(HWND w,style_client_frame *s)
{return (style_menu_bar_target){w,&s->instance,&s->menu,s,&bar_ops};}
static void leave_bar(HWND w,style_client_frame *s,BOOL restore_focus)
{const style_menu_bar_target t=menu_target(w,s);style_menu_bar_finish(&t,restore_focus);}
static BOOL refresh_font(HWND w,void *context)
{style_client_frame *s=context;return style_frame_fonts_refresh(w,&s->fonts,&s->caption_font,&s->menu_font);}

static void activate_frame(HWND w,void *context,const RECT *client,BOOL normal)
{
    style_client_frame *s=context;style_full_client_layout l;
    s->native.keep_outer=normal && client && layout(w,s,&l) && !s->instance.detached && style_frame_keep_outer(w,client,&l.client);
    if(!s->instance.detached)style_native_frame_activate(w,&s->native,&s->instance.detached,client,normal,DwmSetWindowAttribute);
}

static DWORD load_theme(style_client_frame *frame,BOOL use_theme_colors)
{
    classic_theme_mapping source={0};
    DWORD error=classic_theme_attachment_layout((HMODULE)&__ImageBase,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,&source);
    if(error)return error;
    if(source.resources){
        const classic_theme *theme=&source.model;
        frame->menu_font=theme->menu_font;
        frame->menu_metrics=theme->menu_metrics;
        (void)style_menu_art_read(source.resources,theme->menu_images,&frame->menu_art);
        if(use_theme_colors){
            frame->menu_bar=theme->menu_bar;
            frame->menu_popup=theme->menu_popup;
            classic_theme_window_palette(&frame->palette,theme);
        }
        if(!frame->menu_only){
            frame->title_buttons=theme->title_buttons;
            frame->caption_metrics=theme->caption_metrics;
            frame->caption_presentation=theme->caption_presentation;
            frame->border_presentation=theme->border_presentation;
            frame->caption_gradient=theme->gradient;
            frame->active_end=theme->active.end;
            frame->inactive_end=theme->inactive.end;
            frame->caption_font=theme->caption_font;
            (void)classic_caption_art_read(source.resources,&theme->caption_images,&frame->art);
        }
        classic_theme_close(&source);
    }
    if(frame->menu_only)frame->fonts.menu=style_frame_menu_font_acquire(frame->fonts.dpi,&frame->menu_font);
    return ERROR_SUCCESS;
}

static const style_client_frame_lifecycle_services frame_lifecycle={style_proc,DwmGetWindowAttribute,DwmSetWindowAttribute,local_window,state,load_theme,refresh_font,layout,activate_frame,
    win16_style_frame_scrollbars_install_owned,win16_style_control_remove_owned,win16_style_remove,leave_bar};
BOOL win16_style_menu_owner_install_created(HWND w,const void *owner,const BOOL *cancelled)
{return style_client_frame_menu_owner_install(w,owner,cancelled,&frame_lifecycle);}
#ifdef NTWSHK_UNIFIED
BOOL ntw16s_explicit_install_allowed(HWND window,const win16_style_palette *palette)
{
    if(!local_window(window))return FALSE;
    if(palette){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    return classic_theme_require_layout(CLASSIC_WINDOW_PROVIDER_FULL_CLIENT);
}
#endif
BOOL win16_style_install(HWND w,const win16_style_palette *p)
{
#ifdef NTWSHK_UNIFIED
    if(!ntw16s_explicit_install_allowed(w,p))return FALSE;
#endif
    return style_client_frame_install(w,p,FALSE,NULL,NULL,&frame_lifecycle);
}
BOOL win16_style_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_client_frame_install(w,p,TRUE,owner,cancelled,&frame_lifecycle);}
BOOL win16_style_frame_remove_owned(HWND w,const void *owner)
{return style_client_frame_remove_owned(w,owner,&frame_lifecycle);}
BOOL win16_style_remove(HWND w)
{return style_client_frame_remove(w,&frame_lifecycle);}
#ifndef NTWSHK_UNIFIED
BOOL win16_style_set_palette(HWND w,const win16_style_palette *p)
{
    if(!p){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!local_window(w))return FALSE;
    style_client_frame *frame=state(w);
    if(!frame){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_instance_retain(&frame->instance);
    frame->palette=*p;
    win16_style_frame_scrollbars_palette_owned(w,p,frame);
    paint(w,frame,NULL);
    style_instance_release(&frame->instance);
    return TRUE;
}
#endif
BOOL win16_style_frame_terminal(HWND w)
{return style_client_frame_terminal(w,&frame_lifecycle);}

static LRESULT dispatch_frame(void *context,HWND w,UINT msg,WPARAM wp,LPARAM lp)
{return style_proc_body(w,msg,wp,lp,0,(DWORD_PTR)context);}
static void finish_menu_dispatch(void *context,BOOL completed)
{if(!completed)style_instance_release(&((style_client_frame *)context)->instance);}
static LRESULT CALLBACK style_proc(HWND w,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    style_client_frame *s=(style_client_frame *)data;
    if(!s->menu_only)return style_proc_body(w,msg,wp,lp,id,data);
    const style_dispatch_ops ops={s,s,dispatch_frame,finish_menu_dispatch};
    return style_dispatch_message(w,msg,wp,lp,&ops);
}
static BOOL measure_client(HWND w,void *context,const RECT *outer,RECT *client)
{
    style_client_frame *s=context;style_full_client_layout layout;
    if(!style_client_frame_measure_window(w,&s->instance,&s->fonts,outer,&s->title_buttons,&s->caption_metrics,
        &s->border_presentation.geometry,s->caption_presentation.icon_mode,&s->menu_metrics,&layout))
        return FALSE;
    *client=layout.client;return TRUE;
}
static BOOL frame_hit(HWND w,void *context,LPARAM point,LRESULT *result)
{(void)context;return win16_style_hit_test(w,point,result);}
static LRESULT CALLBACK style_proc_body(HWND w,UINT msg,WPARAM wp,LPARAM lp,
    UINT_PTR id,DWORD_PTR data)
{
    (void)id;
    const style_frame_message_ops message={(void *)data,measure_client,activate_frame,paint,paint_buffered_callback,frame_hit,refresh_font};
    const style_client_frame_input_services input={layout,menu_target,button_view};
    const style_client_frame_dispatch_services services={&message,&input,paint_button_state,win16_style_frame_terminal};
    return style_client_frame_dispatch(w,(style_client_frame *)data,msg,wp,lp,&services);
}
