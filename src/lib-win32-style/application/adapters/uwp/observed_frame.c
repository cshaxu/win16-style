#include "observed_frame.h"
#include "lib-win32-style/application/menu/native_system_menu.h"
#include "lib-win32-style/application/adapters/surface.h"
#include "lib-win32-style/application/window/window_icon.h"
#include <stdlib.h>

typedef struct modern_frame {
    style_uwp_frame frame;
    RECT bounds,client;
    style_native_caption_layout caption;
    UINT dpi;
    BOOL maximized,sizeable;
    HFONT font;
    style_uwp_observed_theme theme;
    style_uwp_frame_action_entry actions[];
} modern_frame;

static BOOL geometry(void *context,RECT *outer,RECT *hole,POINT *anchor)
{
    modern_frame *s=context;
    HWND owner=s->frame.host.owner;
    if(!style_uwp_frame_bounds(owner,s->frame.host.title,outer,&s->client,&s->maximized))return FALSE;
    s->bounds=(RECT){0,0,outer->right-outer->left,outer->bottom-outer->top};
    if(IsRectEmpty(&s->client) || s->client.left<0 || s->client.right>s->bounds.right ||
       s->client.bottom>s->bounds.bottom)return FALSE;
    DWORD style=(DWORD)GetWindowLongPtrW(owner,GWL_STYLE);
    DWORD ex=(DWORD)GetWindowLongPtrW(owner,GWL_EXSTYLE);
    if((style&WS_CAPTION)!=WS_CAPTION || (ex&(WS_EX_LAYOUTRTL|WS_EX_TOOLWINDOW)))return FALSE;
    s->sizeable=!s->maximized && (style&WS_THICKFRAME);
    UINT dpi=GetDpiForWindow(owner);
    if(!dpi || s->frame.host.instance.detached)return FALSE;
    const style_uwp_observed_theme *theme=&s->theme;
    const style_frame_geometry *profile=theme->present?&theme->border.geometry:NULL;
    enum style_frame_border_kind kind=s->sizeable?STYLE_FRAME_SIZEABLE:
        ex&WS_EX_DLGMODALFRAME?STYLE_FRAME_DIALOG:STYLE_FRAME_FIXED;
    int x=s->maximized?0:style_frame_window_border_width(owner,profile,kind,TRUE,dpi);
    int y=s->maximized?0:style_frame_window_border_width(owner,profile,kind,FALSE,dpi);
    int height=style_native_caption_measure_height(dpi,FALSE,theme->present?&theme->metrics:NULL,
        profile?profile->caption_floor:0);
    s->caption.band=(RECT){x,y,s->bounds.right-x,y+height};
    int separator=profile?profile->separator_units*max(1,MulDiv(1,(int)dpi,96)):0;
    if(s->caption.band.bottom+separator>=s->client.top || s->caption.band.right<=s->caption.band.left)return FALSE;
    BOOL system=(style&WS_SYSMENU)!=0;
    BOOL present[]={system && (style&WS_MINIMIZEBOX),system && (style&WS_MAXIMIZEBOX),
        system && (ex&WS_EX_CONTEXTHELP),system};
    SetRectEmpty(&s->caption.system);
    BOOL measured=theme->default_buttons?
        theme->default_buttons(&s->caption.band,dpi,FALSE,present,s->caption.buttons):
        style_native_caption_buttons_with_system(&s->caption.band,dpi,FALSE,present,
            &theme->buttons,&theme->metrics,s->caption.buttons,system?&s->caption.system:NULL);
    if(!measured)return FALSE;
    s->caption.left_buttons=theme->present?style_native_caption_left_buttons(&theme->buttons):0;
    s->caption.system_on_right=FALSE;
    if(!IsRectEmpty(&s->caption.system))
        (void)style_native_caption_system_side(&theme->buttons,&s->caption.system_on_right);
    SetRectEmpty(&s->caption.icon);
    if(style_caption_icon_visible(theme->caption.icon_mode,
        !IsRectEmpty(&s->caption.system),style,ex)){
        if(!style_caption_icon_bounds(s->caption.band,s->caption.band.right,dpi,&s->caption.icon))return FALSE;
        for(unsigned i=0;i<ARRAYSIZE(s->caption.buttons);++i)
            if(!IsRectEmpty(&s->caption.buttons[i]) && s->caption.buttons[i].left<=s->caption.icon.right)return FALSE;
    }
    if(s->dpi!=dpi){
        HFONT font=theme->create_font(theme->present?&theme->font:NULL,dpi);
        if(!font)return FALSE;
        if(s->font)DeleteObject(s->font);
        s->font=font;s->dpi=dpi;
    }
    *hole=s->client;
    RECT menu=IsRectEmpty(&s->caption.system)?s->caption.band:s->caption.system;
    *anchor=(POINT){menu.left,menu.bottom};
    return TRUE;
}

static void paint(void *context,HDC dc)
{
    modern_frame *s=context;
    HWND owner=s->frame.host.owner;
    WCHAR title[1024]=L"";
    GetWindowTextW(owner,title,ARRAYSIZE(title));
    if(s->frame.host.instance.detached)return;
    int saved=SaveDC(dc);
    if(!saved)return;
    const style_uwp_observed_theme *theme=&s->theme;
    const style_frame_colors *colors=theme->present?&theme->colors:NULL;
    SetDCBrushColor(dc,style_native_frame_colors(colors).face);
    FillRect(dc,&s->bounds,GetStockObject(DC_BRUSH));
    if(!s->maximized && theme->present)
        style_native_frame_border_paint(dc,s->bounds,s->caption.band.top,&theme->border,colors,
            (s->frame.active?theme->active:theme->inactive).left,
            (GetWindowLongPtrW(owner,GWL_EXSTYLE)&WS_EX_DLGMODALFRAME)!=0);
    UINT flags[STYLE_NATIVE_CAPTION_BUTTONS]={0};
    if(!IsRectEmpty(&s->caption.buttons[STYLE_NATIVE_CAPTION_CLOSE]) && !style_caption_button_enabled(owner,HTCLOSE))
        flags[STYLE_NATIVE_CAPTION_CLOSE]|=DFCS_INACTIVE;
    if(s->frame.hot){
        unsigned index=style_caption_role(s->frame.pressed)-CLASSIC_BUTTON_MINIMIZE;
        if(index<ARRAYSIZE(flags))flags[index]|=DFCS_PUSHED;
    }
    s->caption.system_flags=s->frame.no_menu ||
        (s->frame.hot && s->frame.pressed==HTSYSMENU)?DFCS_PUSHED:0;
    HICON icon=!s->frame.host.instance.detached && !IsRectEmpty(&s->caption.icon)?
        style_window_caption_icon(owner):NULL;
    if(!s->frame.host.instance.detached)
        style_native_caption_render(dc,&s->caption,title,icon,s->font,s->dpi,
            theme->present?(s->frame.active?theme->active:theme->inactive):style_native_caption_palette(s->frame.active),colors,
            theme->present?&theme->caption:NULL,s->maximized,flags,&theme->art);
    RestoreDC(dc,saved);
}

static LRESULT hit(void *context,POINT point)
{
    modern_frame *s=context;
    if(PtInRect(&s->client,point))return HTCLIENT;
    if(!PtInRect(&s->bounds,point))return HTNOWHERE;
    int part=style_caption_hit(s->caption.buttons,CLASSIC_BUTTON_MINIMIZE,~0u,point);
    if(part)return part;
    if(PtInRect(&s->caption.icon,point) || PtInRect(&s->caption.system,point))return HTSYSMENU;
    if(PtInRect(&s->caption.band,point))return HTCAPTION;
    if(s->sizeable && point.y<s->caption.band.top && point.x>=s->client.left && point.x<s->client.right)
        return HTTOP;
    const style_uwp_observed_theme *theme=&s->theme;
    int width=theme->present?style_caption_button_width(&theme->metrics,s->dpi,FALSE):
        MulDiv(theme->default_grip_width,(int)s->dpi,96);
    SIZE corner={s->caption.band.left+width,s->client.top};
    return style_uwp_resize_margin_hit(HTBORDER,point,&s->bounds,&s->client,corner,s->sizeable);
}

static BOOL can_maximize(void *context)
{
    modern_frame *s=context;
    return !IsRectEmpty(&s->caption.buttons[STYLE_NATIVE_CAPTION_MAXIMIZE]);
}

static BOOL menu(void *context,HWND window,UINT message,WPARAM a,LPARAM b,BOOL owner,LRESULT *result)
{
    (void)owner;
    modern_frame *s=context;
    if(message==WM_INITMENUPOPUP && s->frame.tracking && (HMENU)a==GetSystemMenu(s->frame.host.owner,FALSE))
        b=MAKELPARAM(LOWORD(b),TRUE);
    return style_menu_owner_message(&s->frame.menu,window,message,a,b,&style_native_system_menu_ops,
        s->theme.present?&s->theme.menu:NULL,result);
}

static void dispose(void *context)
{
    modern_frame *s=context;
    if(s->font)DeleteObject(s->font);
    free(s);
}

static const style_uwp_frame_ops ops={geometry,paint,hit,can_maximize,menu,dispose};


BOOL style_uwp_observed_frame_apply(HWND owner,HWND title,const style_uwp_frame_action *actions,unsigned count,
    const style_uwp_observed_theme *theme)
{
    if(!title){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!style_uwp_frame_validate(owner,title,actions,count))return FALSE;
    modern_frame *s=calloc(1,sizeof(*s)+(size_t)count*sizeof(*s->actions));
    if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->theme=*theme;
    return style_uwp_frame_start(&s->frame,owner,title,&ops,s,actions,count,s->actions);
}
