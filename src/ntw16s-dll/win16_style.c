/* Project-authored experimental non-client presentation, not imported USER.
 * See T445 S1 provenance/design: original NTUSER server is a stopping boundary.
 */
#include "win16_style.h"
#include "lib-win32-style/appearance/win3x/common/layout.h"
#include "../lib-win32-style/appearance/win3x/window/frame_paint.h"
#include "../lib-win32-style/appearance/win3x/window/frame_renderer.h"
#include "../lib-win32-style/appearance/win3x/window/frame_state.h"
#include "../lib-win32-style/appearance/win3x/window/frame_lifecycle.h"
#include "lib-win32-style/application/menu/bar_controller.h"
#include "lib-win32-style/application/window/messages.h"
#include "../lib-win32-style/appearance/win3x/window/frame_dispatch.h"
#include "../lib-win32-style/appearance/win3x/window/frame_input.h"
#include "../lib-win32-style/appearance/win3x/window/frame_layout.h"
#include "../lib-win32-style/appearance/win3x/window/frame_font.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/window/frame.h"
#include "lib-win32-style/application/window/window_identity.h"
#include "../lib-win32-style/appearance/win3x/menu/menu_bar.h"
#include "../lib-win32-style/appearance/win3x/menu/menu_items.h"
#include "../lib-win32-style/appearance/win3x/menu/menu_scope.h"
#include "controls.h"
#include "../lib-win32-style/appearance/win3x/common/font.h"
#include "lib-win32-style/application/lifecycle/dispatch.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <windowsx.h>
#include <string.h>

static LRESULT CALLBACK style_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK style_proc_body(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

/* -3 means outside the bar/system cell; -2 denotes the system cell. */
static style_window *state(HWND w)
{
    return (style_window *)style_instance_lookup(w,style_proc);
}

static BOOL local_window(HWND w)
{
    if(!win32_style_window_current_thread(w)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return TRUE;
}

static BOOL layout(HWND w,style_window *s,win16_style_layout *l)
{
    RECT r;
    return GetWindowRect(w,&r) && win16_control_measure_window(w,&s->instance,&s->fonts,&r,
        &s->title_buttons,&s->caption_metrics,&s->border_presentation.geometry,l);
}

BOOL win16_style_hit_test(HWND w,LPARAM lp,LRESULT *result)
{
    style_window *s;win16_style_layout l;
    if(!result || !local_window(w))return FALSE;
    s=state(w);
    if(!s || !layout(w,s,&l))return FALSE;
    return win16_control_frame_hit_test(w,&l,lp,result);
}

static BOOL paint_layout(HWND w,void *context,win16_style_layout *result)
{return layout(w,(style_window *)context,result);}
static void paint_colors(void *context,win16_style_palette *palette)
{win16_control_resolve_palette(&((style_window *)context)->palette,palette);}
static void paint_state(void *context,win16_frame_paint_state *result)
{
    style_window *s=context;
    *result=(win16_frame_paint_state){&s->fonts,s->native.active,s->menu.system,
        s->menu.bar.keyboard,s->menu.bar.selected,s->button,&s->art,&s->menu_bar,
        s->caption_presentation,s->caption_gradient,s->native.active?s->active_end:s->inactive_end,
        s->border_presentation};
}
static const win16_frame_renderer_ops renderer={paint_layout,paint_state,paint_colors,win16_style_scrollbars_paint};
static void paint(HWND w,void *context,HDC target)
{style_window *s=context;win16_control_paint_frame(w,&s->instance,s,&renderer,target);}
static void paint_buffered_callback(HWND w,void *context,HDC dc)
{style_window *s=context;win16_control_frame_buffered(w,&s->instance,s,&renderer,dc);}
static void paint_menu_state(HWND w,style_window *s)
{win16_control_frame_menu_state(w,&s->instance,s,&renderer);}
static void paint_button_state(HWND w,style_window *s,int hit)
{win16_control_frame_button_state(w,&s->instance,s,&renderer,hit);}
static void button_paint_callback(HWND w,void *context,int hit){paint_button_state(w,(style_window *)context,hit);}
static win16_control_frame_button_view button_view(style_window *s)
{
    const win16_control_frame_button_view view={s,&s->button,&s->instance,paint_layout,button_paint_callback};
    return view;
}

static BOOL bar_read(const style_menu_bar_target *target,style_menu_bar_snapshot *bar)
{
    style_window *s=target->context;win16_style_layout l;RECT outer;
    if(!layout(target->window,s,&l) || !GetWindowRect(target->window,&outer))return FALSE;
    if(!win16_style_bar_read_for_window(target->window,GetMenu(target->window),s->fonts.menu,s->fonts.dpi,
        (POINT){l.menu.left,l.menu.top},l.menu.right-l.menu.left,bar))return FALSE;
    for(UINT i=0;i<bar->count;++i)OffsetRect(&bar->rects[i],outer.left,outer.top);
    return TRUE;
}
static int bar_hit(const style_menu_bar_target *target,POINT point)
{
    style_window *s=target->context;win16_style_layout l;
    if(s->instance.detached || !layout(target->window,s,&l))return STYLE_MENU_NO_ITEM;
    return win16_control_menu_bar_hit_screen(target->window,&l,s->fonts.menu,s->fonts.dpi,point);
}
static void bar_paint(const style_menu_bar_target *target)
{paint_menu_state(target->window,target->context);}
static BOOL bar_target(HWND,style_menu_bar_target *);
static const style_menu_bar_ops bar_ops={bar_read,bar_hit,bar_paint,bar_target,STYLE_MENU_TRACK_COLUMNS};
static BOOL bar_target(HWND w,style_menu_bar_target *target)
{
    style_window *s=state(w);if(!s)return FALSE;
    *target=(style_menu_bar_target){w,&s->instance,&s->menu,s,&bar_ops};return TRUE;
}
static style_menu_bar_target menu_target(HWND w,style_window *s)
{return (style_menu_bar_target){w,&s->instance,&s->menu,s,&bar_ops};}
static void leave_bar(HWND w,style_window *s,BOOL restore_focus)
{const style_menu_bar_target t=menu_target(w,s);style_menu_bar_finish(&t,restore_focus);}
static BOOL refresh_font(HWND w,void *context)
{style_window *s=context;return win16_control_refresh_frame_fonts(w,&s->fonts,&s->caption_font,&s->menu_font);}

static void activate_frame(HWND w,void *context,const RECT *client,BOOL normal)
{
    style_window *s=context;win16_style_layout l;
    s->native.keep_outer=normal && client && layout(w,s,&l) && !s->instance.detached && style_frame_keep_outer(w,client,&l.client);
    if(!s->instance.detached)style_native_frame_activate(w,&s->native,&s->instance.detached,client,normal,DwmSetWindowAttribute);
}

static const win16_control_frame_lifecycle_services frame_lifecycle={style_proc,DwmGetWindowAttribute,DwmSetWindowAttribute,local_window,state,refresh_font,layout,activate_frame,
    win16_style_frame_scrollbars_install_owned,win16_style_frame_scrollbars_palette_owned,win16_style_control_remove_owned,
    win16_style_remove,leave_bar,paint};
BOOL win16_style_menu_owner_install_created(HWND w,const void *owner,const BOOL *cancelled)
{return win16_control_menu_owner_install(w,owner,cancelled,&frame_lifecycle);}
BOOL win16_style_install(HWND w,const win16_style_palette *p)
{return win16_control_frame_install(w,p,FALSE,NULL,NULL,&frame_lifecycle);}
BOOL win16_style_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_frame_install(w,p,TRUE,owner,cancelled,&frame_lifecycle);}
BOOL win16_style_frame_remove_owned(HWND w,const void *owner)
{return win16_control_frame_remove_owned(w,owner,&frame_lifecycle);}
BOOL win16_style_remove(HWND w)
{return win16_control_frame_remove(w,&frame_lifecycle);}
BOOL win16_style_set_palette(HWND w,const win16_style_palette *p)
{return win16_control_frame_set_palette(w,p,&frame_lifecycle);}
BOOL win16_style_frame_terminal(HWND w)
{return win16_control_frame_terminal(w,&frame_lifecycle);}

static LRESULT dispatch_frame(void *context,HWND w,UINT msg,WPARAM wp,LPARAM lp)
{return style_proc_body(w,msg,wp,lp,0,(DWORD_PTR)context);}
static void finish_menu_dispatch(void *context,BOOL completed)
{if(!completed)style_instance_release(&((style_window *)context)->instance);}
static LRESULT CALLBACK style_proc(HWND w,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    style_window *s=(style_window *)data;
    if(!s->menu_only)return style_proc_body(w,msg,wp,lp,id,data);
    const style_dispatch_ops ops={s,s,dispatch_frame,finish_menu_dispatch};
    return style_dispatch_message(w,msg,wp,lp,&ops);
}
static BOOL measure_client(HWND w,void *context,const RECT *outer,RECT *client)
{
    style_window *s=context;win16_style_layout layout;
    if(!win16_control_measure_window(w,&s->instance,&s->fonts,outer,&s->title_buttons,&s->caption_metrics,
        &s->border_presentation.geometry,&layout))
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
    const win16_control_frame_input_services input={layout,menu_target,button_view};
    const win16_control_frame_dispatch_services services={&message,&input,paint_button_state,win16_style_frame_terminal};
    return win16_control_frame_dispatch(w,(style_window *)data,msg,wp,lp,&services);
}
