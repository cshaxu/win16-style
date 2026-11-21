#include "modern_frame.h"
#include "caption.h"
#include "system_menu.h"
#include "lib-win32-style/application/adapters/surface.h"
#include "lib-win32-style/application/window/window_icon.h"
#include <stdlib.h>

typedef struct modern_frame {
    style_uwp_frame frame;
    RECT bounds,client;
    win2k_caption_layout caption;
    UINT dpi;
    BOOL maximized,sizeable;
#ifndef STYLE_THEME_EXTERNAL_ONLY
    BOOL themed;
#endif
    HFONT font;
    classic_theme theme;
    classic_caption_art art;
    win2k_menu_style menu_style;
    style_uwp_frame_action_entry actions[];
} modern_frame;

static const classic_theme *active_theme(const modern_frame *frame)
{
#ifndef STYLE_THEME_EXTERNAL_ONLY
    if(!frame->themed)return NULL;
#endif
    return &frame->theme;
}

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
    const classic_theme *theme=active_theme(s);
    const style_frame_geometry *profile=theme?&theme->border_presentation.geometry:NULL;
    enum style_frame_border_kind kind=s->sizeable?STYLE_FRAME_SIZEABLE:
        ex&WS_EX_DLGMODALFRAME?STYLE_FRAME_DIALOG:STYLE_FRAME_FIXED;
    int x=s->maximized?0:style_frame_window_border_width(owner,profile,kind,TRUE,dpi);
    int y=s->maximized?0:style_frame_window_border_width(owner,profile,kind,FALSE,dpi);
    int height=win2k_caption_theme_height(dpi,FALSE,theme);
    s->caption.band=(RECT){x,y,s->bounds.right-x,y+height};
    int separator=profile?profile->separator_units*max(1,MulDiv(1,(int)dpi,96)):0;
    if(s->caption.band.bottom+separator>=s->client.top || s->caption.band.right<=s->caption.band.left)return FALSE;
    BOOL system=(style&WS_SYSMENU)!=0;
    BOOL present[]={system && (style&WS_MINIMIZEBOX),system && (style&WS_MAXIMIZEBOX),
        system && (ex&WS_EX_CONTEXTHELP),system};
    SetRectEmpty(&s->caption.system);
    BOOL measured;
#ifndef STYLE_THEME_EXTERNAL_ONLY
    if(!theme)measured=win2k_caption_system_buttons(&s->caption.band,dpi,FALSE,present,s->caption.buttons);
    else
#endif
    measured=win2k_caption_buttons_with_system(&s->caption.band,dpi,FALSE,present,
        &theme->title_buttons,&theme->caption_metrics,s->caption.buttons,system?&s->caption.system:NULL);
    if(!measured)return FALSE;
    s->caption.left_buttons=theme?win2k_caption_left_buttons(&theme->title_buttons):0;
    s->caption.system_on_right=FALSE;
    if(!IsRectEmpty(&s->caption.system))
        (void)win2k_caption_system_side(&theme->title_buttons,&s->caption.system_on_right);
    SetRectEmpty(&s->caption.icon);
    if(style_caption_icon_visible(theme?theme->caption_presentation.icon_mode:0,
        !IsRectEmpty(&s->caption.system),style,ex)){
        if(!style_caption_icon_bounds(s->caption.band,s->caption.band.right,dpi,&s->caption.icon))return FALSE;
        for(unsigned i=0;i<ARRAYSIZE(s->caption.buttons);++i)
            if(!IsRectEmpty(&s->caption.buttons[i]) && s->caption.buttons[i].left<=s->caption.icon.right)return FALSE;
    }
    if(s->dpi!=dpi){
        HFONT font=msstyles_font_create(theme?&theme->caption_font:NULL,dpi,MSSTYLES_FONT_CAPTION);
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
    const classic_theme *theme=active_theme(s);
    const classic_frame_colors *colors=theme?&theme->frame:NULL;
    SetDCBrushColor(dc,win2k_window_frame_colors(colors).face);
    FillRect(dc,&s->bounds,GetStockObject(DC_BRUSH));
    if(!s->maximized)win2k_window_border_paint(dc,s->bounds.right,s->bounds.bottom,s->caption.band.top,
        theme,s->frame.active,(GetWindowLongPtrW(owner,GWL_EXSTYLE)&WS_EX_DLGMODALFRAME)!=0);
    UINT flags[WIN2K_CAPTION_PARTS]={0};
    if(!IsRectEmpty(&s->caption.buttons[WIN2K_CLOSE]) && !style_caption_button_enabled(owner,HTCLOSE))
        flags[WIN2K_CLOSE]|=DFCS_INACTIVE;
    if(s->frame.hot){
        unsigned index=style_caption_role(s->frame.pressed)-CLASSIC_BUTTON_MINIMIZE;
        if(index<ARRAYSIZE(flags))flags[index]|=DFCS_PUSHED;
    }
    s->caption.system_flags=s->frame.no_menu ||
        (s->frame.hot && s->frame.pressed==HTSYSMENU)?DFCS_PUSHED:0;
    HICON icon=!s->frame.host.instance.detached && !IsRectEmpty(&s->caption.icon)?
        style_window_caption_icon(owner):NULL;
    if(!s->frame.host.instance.detached)
        win2k_caption_render_with_colors(dc,&s->caption,title,icon,s->font,s->dpi,
            win2k_caption_theme_palette(s->frame.active,theme),colors,
            theme?&theme->caption_presentation:NULL,s->maximized,flags,&s->art);
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
    SIZE corner={s->caption.band.left+MulDiv(16,(int)s->dpi,96),s->client.top};
    return style_uwp_resize_margin_hit(HTBORDER,point,&s->bounds,&s->client,corner,s->sizeable);
}

static BOOL can_maximize(void *context)
{
    modern_frame *s=context;
    return !IsRectEmpty(&s->caption.buttons[WIN2K_MAXIMIZE]);
}

static BOOL menu(void *context,HWND window,UINT message,WPARAM a,LPARAM b,BOOL owner,LRESULT *result)
{
    (void)owner;
    modern_frame *s=context;
    if(message==WM_INITMENUPOPUP && s->frame.tracking && (HMENU)a==GetSystemMenu(s->frame.host.owner,FALSE))
        b=MAKELPARAM(LOWORD(b),TRUE);
    return style_menu_owner_message(&s->frame.menu,window,message,a,b,&win2k_system_menu_operations,
        active_theme(s)?&s->menu_style:NULL,result);
}

static void dispose(void *context)
{
    modern_frame *s=context;
    if(s->font)DeleteObject(s->font);
    free(s);
}

static const style_uwp_frame_ops ops={geometry,paint,hit,can_maximize,menu,dispose};


BOOL win2k_uwp_frame_apply(HWND owner,HWND title,const style_uwp_frame_action *actions,unsigned count,
    const classic_theme_mapping *source)
{
    if(!title){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
#ifdef STYLE_THEME_EXTERNAL_ONLY
    if(!source || !source->resources){SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);return FALSE;}
#endif
    if(!style_uwp_frame_validate(owner,title,actions,count))return FALSE;
    modern_frame *s=calloc(1,sizeof(*s)+(size_t)count*sizeof(*s->actions));
    if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
#ifndef STYLE_THEME_EXTERNAL_ONLY
    s->themed=source!=NULL;
    if(source)
#endif
    {
        s->theme=source->model;
        win2k_menu_style_read(&s->menu_style,source);
        if(source->resources)
            (void)classic_caption_art_read(source->resources,&s->theme.caption_images,&s->art);
    }
    return style_uwp_frame_start(&s->frame,owner,title,&ops,s,actions,count,s->actions);
}
