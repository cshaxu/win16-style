#include "client_frame.h"
#include "client_frame_layout.h"
#include "lib-win32-style/application/adapters/surface.h"
#include "lib-win32-style/application/window/client_frame_renderer.h"
#include "lib-win32-style/application/menu/owned_rows.h"
#include <stdlib.h>

typedef struct uwp_frame {
    style_uwp_frame frame;
    style_font_pair fonts;
    style_full_client_layout layout;
    style_uwp_client_theme theme;
    style_uwp_frame_action_entry actions[];
} uwp_frame;

static void dispose(void *context)
{
    uwp_frame *s=context;
    style_font_pair_dispose(&s->fonts,style_frame_fonts_release);
    free(s);
}
static BOOL geometry(uwp_frame *s,RECT *outer)
{
    RECT content;BOOL zoomed;
    if(!style_uwp_frame_bounds(s->frame.host.owner,s->frame.host.title,outer,&content,&zoomed))return FALSE;
    SIZE size={outer->right-outer->left,outer->bottom-outer->top};
    UINT dpi=GetDpiForWindow(s->frame.host.owner);
    DWORD style=(DWORD)GetWindowLongPtrW(s->frame.host.owner,GWL_STYLE);
    DWORD ex_style=(DWORD)GetWindowLongPtrW(s->frame.host.owner,GWL_EXSTYLE);
    if(!style_uwp_client_frame_layout(size,dpi,style,ex_style,zoomed,content,
        &s->theme.title_buttons,&s->theme.caption_metrics,&s->theme.border_presentation.geometry,&s->layout))return FALSE;
    style_full_client_caption_icon(&s->layout,dpi,style,ex_style,s->theme.caption_presentation.icon_mode);
    if(dpi!=s->fonts.dpi && !s->theme.refresh_fonts(s->frame.host.owner,&s->fonts,
        &s->theme.caption_font,&s->theme.menu_font))return FALSE;
    return !s->frame.host.instance.detached;
}
static BOOL cached_layout(HWND w,void *context,style_full_client_layout *layout)
{(void)w;*layout=((uwp_frame *)context)->layout;return TRUE;}
static void colors(void *context,style_window_palette *palette)
{style_window_palette_resolve(&((uwp_frame *)context)->theme.palette,palette);}
static void no_scrollbars(HWND w,HDC dc,const style_window_palette *palette)
{(void)w;(void)dc;(void)palette;}
static BOOL refresh_geometry(void *context,RECT *outer,RECT *hole,POINT *anchor)
{
    uwp_frame *s=context;
    if(!geometry(s,outer))return FALSE;
    RECT button=s->layout.buttons[CLASSIC_BUTTON_SYSTEM];
    if(IsRectEmpty(&button))button=s->layout.icon;
    if(IsRectEmpty(&button))button=s->layout.caption;
    *hole=s->layout.client;*anchor=(POINT){button.left,button.bottom};return TRUE;
}
static void paint_state(void *context,style_client_frame_paint_state *result)
{
    uwp_frame *s=context;
    *result=(style_client_frame_paint_state){&s->fonts,s->frame.active,s->frame.no_menu,FALSE,-1,
        {s->frame.pressed,s->frame.hot},&s->theme.art,&s->theme.menu_bar,
        s->theme.caption_presentation,s->theme.gradient,s->frame.active?s->theme.active_end:s->theme.inactive_end,
        s->theme.border_presentation,&s->theme.menu_metrics,FALSE,NULL};
}
static const style_client_frame_renderer_ops renderer={cached_layout,paint_state,colors,no_scrollbars};
static void refresh_paint(void *context,HDC dc)
{
    uwp_frame *s=context;
    style_client_frame_paint(s->frame.host.owner,&s->frame.host.instance,s,&renderer,dc);
}
static LRESULT input_hit(void *context,POINT point)
{return style_uwp_client_frame_hit(&((uwp_frame *)context)->layout,point);}
static BOOL can_maximize(void *context)
{
    uwp_frame *s=context;
    return (GetWindowLongPtrW(s->frame.host.owner,GWL_STYLE)&WS_MAXIMIZEBOX)!=0;
}
static BOOL menu_message(void *context,HWND w,UINT message,WPARAM a,LPARAM b,BOOL owner,LRESULT *result)
{
    uwp_frame *s=context;
    style_menu_client_theme theme={&s->theme.palette,s->fonts.menu,s->fonts.dpi,
        owner?&s->frame:NULL,owner?style_uwp_frame_menu_selection:NULL,
        &s->theme.menu_popup,&s->theme.menu_metrics,&s->theme.menu_art};
    return style_menu_owner_message(&s->frame.menu,w,message,a,b,&style_menu_client_operations,&theme,result);
}
static const style_uwp_frame_ops operations={refresh_geometry,refresh_paint,input_hit,can_maximize,menu_message,dispose};
BOOL style_uwp_client_frame_apply(HWND w,HWND title,const style_uwp_frame_action *actions,unsigned count,
    const style_uwp_client_theme *theme)
{
    if(!style_uwp_frame_validate(w,title,actions,count))return FALSE;
    uwp_frame *s=calloc(1,sizeof(*s)+(size_t)count*sizeof(*s->actions));
    if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->theme=*theme;
    return style_uwp_frame_start(&s->frame,w,title,&operations,s,actions,count,s->actions);
}
