#include "modern_frame.h"
#include "modern_frame_layout.h"
#include "lib-win32-style/application/adapters/surface.h"
#include "lib-win32-style/appearance/win3x/window/frame_renderer.h"
#include "lib-win32-style/appearance/win3x/window/frame_font.h"
#include "lib-win32-style/appearance/win3x/common/font.h"
#include "lib-win32-style/appearance/win3x/menu/menu_items.h"
#include <stdlib.h>

extern IMAGE_DOS_HEADER __ImageBase;

typedef struct uwp_frame {
    style_uwp_frame frame;
    style_font_pair fonts;
    win16_style_layout layout;
    classic_theme theme;
    win16_style_palette palette;
    win16_menu_art menu_art;
    classic_caption_art caption_art;
    style_uwp_frame_action_entry actions[];
} uwp_frame;

static void dispose(void *context)
{
    uwp_frame *s=context;
    style_font_pair_dispose(&s->fonts,win16_control_release_frame_fonts);
    free(s);
}
static BOOL geometry(uwp_frame *s,RECT *outer)
{
    RECT content;BOOL zoomed;
    if(!style_uwp_frame_bounds(s->frame.host.owner,s->frame.host.title,outer,&content,&zoomed))return FALSE;
    SIZE size={outer->right-outer->left,outer->bottom-outer->top};
    UINT dpi=GetDpiForWindow(s->frame.host.owner);
    DWORD style=(DWORD)GetWindowLongPtrW(s->frame.host.owner,GWL_STYLE);
    if(!win16_uwp_frame_layout(size,dpi,style,zoomed,content,
        &s->theme.title_buttons,&s->theme.caption_metrics,&s->layout))return FALSE;
    if(dpi!=s->fonts.dpi && !win16_control_refresh_frame_fonts(s->frame.host.owner,&s->fonts,
        &s->theme.caption_font,&s->theme.menu_font))return FALSE;
    return !s->frame.host.instance.detached;
}
static BOOL cached_layout(HWND w,void *context,win16_style_layout *layout)
{(void)w;*layout=((uwp_frame *)context)->layout;return TRUE;}
static void colors(void *context,win16_style_palette *palette)
{*palette=((uwp_frame *)context)->palette;}
static void no_scrollbars(HWND w,HDC dc,const win16_style_palette *palette)
{(void)w;(void)dc;(void)palette;}
static BOOL refresh_geometry(void *context,RECT *outer,RECT *hole,POINT *anchor)
{
    uwp_frame *s=context;
    if(!geometry(s,outer))return FALSE;
    *hole=s->layout.client;*anchor=(POINT){s->layout.buttons[CLASSIC_BUTTON_SYSTEM].left,s->layout.buttons[CLASSIC_BUTTON_SYSTEM].bottom};return TRUE;
}
static void paint_state(void *context,win16_frame_paint_state *result)
{
    uwp_frame *s=context;
    *result=(win16_frame_paint_state){&s->fonts,s->frame.active,s->frame.no_menu,FALSE,-1,
        {s->frame.pressed,s->frame.hot},&s->caption_art,&s->theme.menu_bar,
        s->theme.caption_presentation,s->theme.gradient,s->frame.active?s->theme.active.end:s->theme.inactive.end,
        s->theme.border_presentation};
}
static const win16_frame_renderer_ops renderer={cached_layout,paint_state,colors,no_scrollbars};
static void refresh_paint(void *context,HDC dc)
{
    uwp_frame *s=context;
    win16_control_paint_frame(s->frame.host.owner,&s->frame.host.instance,s,&renderer,dc);
}
static void invoke(uwp_frame *s,int hit)
{
    UINT command=hit==HTMINBUTTON?SC_MINIMIZE:hit==HTMAXBUTTON?
        (IsZoomed(s->frame.host.owner)?SC_RESTORE:SC_MAXIMIZE):SC_KEYMENU;
    PostMessageW(s->frame.host.owner,WM_SYSCOMMAND,command,command==SC_KEYMENU?L' ':0);
}
static LRESULT input_hit(void *context,POINT point)
{return win16_uwp_frame_hit(&((uwp_frame *)context)->layout,point);}
static BOOL input_button(void *context,int hit)
{(void)context;return hit==HTSYSMENU || hit==HTMINBUTTON || hit==HTMAXBUTTON;}
static void input_invoke(void *context,int hit,UINT message)
{
    uwp_frame *s=context;
    if(message==WM_LBUTTONUP)invoke(s,hit);
    else if(message==WM_LBUTTONDBLCLK){
        if(hit==HTSYSMENU)PostMessageW(s->frame.host.owner,WM_SYSCOMMAND,SC_CLOSE,0);
        else if(hit==HTCAPTION && (GetWindowLongPtrW(s->frame.host.owner,GWL_STYLE)&WS_MAXIMIZEBOX))invoke(s,HTMAXBUTTON);
    }else if(message==WM_RBUTTONUP && (hit==HTCAPTION || hit==HTSYSMENU))invoke(s,HTSYSMENU);
}
static BOOL menu_message(void *context,HWND w,UINT message,WPARAM a,LPARAM b,BOOL owner,LRESULT *result)
{
    uwp_frame *s=context;
    win16_style_menu_theme theme={&s->palette,s->fonts.menu,s->fonts.dpi,
        owner?&s->frame:NULL,owner?style_uwp_frame_menu_selection:NULL,
        &s->theme.menu_popup,&s->theme.menu_metrics,&s->menu_art};
    return style_menu_owner_message(&s->frame.menu,w,message,a,b,&win16_style_menu_operations,&theme,result);
}
static const style_uwp_frame_ops operations={refresh_geometry,refresh_paint,input_hit,input_button,input_invoke,menu_message,dispose};
BOOL win16_uwp_frame_uses(HWND w)
{return style_uwp_frame_uses(w,&operations);}
static BOOL apply_actions(HWND w,HWND title,const style_uwp_frame_action *actions,unsigned count)
{
    if(!style_uwp_frame_validate(w,title,actions,count))return FALSE;
    uwp_frame *s=calloc(1,sizeof(*s)+(size_t)count*sizeof(*s->actions));
    if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    win16_style_default_palette(&s->palette);
    classic_theme_mapping source={0};
    if(SUCCEEDED(classic_theme_open_packaged((HMODULE)&__ImageBase,CLASSIC_THEME_WIN16,TRUE,&source))){
        s->theme=source.model;
        win16_style_theme_palette(&s->palette,&s->theme);
        (void)win16_menu_art_read(source.resources,s->theme.menu_images,&s->menu_art);
        (void)classic_caption_art_read(source.resources,&s->theme.caption_images,&s->caption_art);
        classic_theme_close(&source);
    }
    return style_uwp_frame_start(&s->frame,w,title,&operations,s,actions,count,s->actions);
}
BOOL win16_uwp_frame_apply_actions(HWND w,HWND title,const style_uwp_frame_action *actions,unsigned count)
{
    if(!title){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    return apply_actions(w,title,actions,count);
}
BOOL win16_uwp_frame_apply_nonclient(HWND w)
{return apply_actions(w,NULL,NULL,0);}
