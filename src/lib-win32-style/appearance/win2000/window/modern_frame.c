#include "modern_frame.h"
#include "caption.h"
#include "system_menu.h"
#include "lib-win32-style/application/adapters/surface.h"
#include "lib-win32-style/application/window/window_icon.h"
#include <stdlib.h>

static const int hits[]={HTMINBUTTON,HTMAXBUTTON,HTHELP,HTCLOSE};

typedef struct modern_frame {
    style_uwp_frame frame;
    RECT bounds,client;
    win2k_caption_layout caption;
    UINT dpi;
    BOOL maximized,sizeable,themed;
    HFONT font;
    classic_theme theme;
    style_uwp_frame_action_entry actions[];
} modern_frame;

static BOOL button(void *context,int part);

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
    int border=s->maximized?0:4;
    const classic_theme *theme=s->themed?&s->theme:NULL;
    int height=win2k_caption_theme_height(dpi,FALSE,theme);
    s->caption.band=(RECT){border,border,s->bounds.right-border,border+height};
    if(s->caption.band.bottom>=s->client.top || s->caption.band.right<=s->caption.band.left)return FALSE;
    BOOL system=(style&WS_SYSMENU)!=0;
    BOOL present[]={system && (style&WS_MINIMIZEBOX),system && (style&WS_MAXIMIZEBOX),
        system && (ex&WS_EX_CONTEXTHELP),system};
    BOOL measured=theme?win2k_caption_buttons_with_group(&s->caption.band,dpi,FALSE,present,
        &theme->title_buttons,&theme->caption_metrics,s->caption.buttons):
        win2k_caption_buttons(&s->caption.band,dpi,FALSE,present,s->caption.buttons);
    if(!measured)return FALSE;
    SetRectEmpty(&s->caption.icon);
    if(system){
        s->caption.icon=s->caption.band;
        s->caption.icon.right=s->caption.icon.left+GetSystemMetricsForDpi(SM_CXSMICON,dpi)+MulDiv(4,(int)dpi,96);
        for(unsigned i=0;i<ARRAYSIZE(s->caption.buttons);++i)
            if(present[i] && s->caption.buttons[i].left<=s->caption.icon.right)return FALSE;
    }
    if(s->dpi!=dpi){
        HFONT font=win2k_caption_font(dpi,FALSE,theme?&theme->caption_font:NULL);
        if(!font)return FALSE;
        if(s->font)DeleteObject(s->font);
        s->font=font;s->dpi=dpi;
    }
    *hole=s->client;
    *anchor=(POINT){s->caption.band.left,s->caption.band.bottom};
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
    const classic_frame_colors *colors=s->themed?&s->theme.frame:NULL;
    SetDCBrushColor(dc,colors && colors->present?colors->face:RGB(212,208,200));
    FillRect(dc,&s->bounds,GetStockObject(DC_BRUSH));
    if(!s->maximized)win2k_window_border_paint(dc,s->bounds.right,s->bounds.bottom,s->caption.band.top,colors);
    UINT flags[WIN2K_CAPTION_PARTS]={0};
    if(!IsRectEmpty(&s->caption.buttons[WIN2K_CLOSE]) && !button(s,HTCLOSE))
        flags[WIN2K_CLOSE]|=DFCS_INACTIVE;
    if(s->frame.hot){
        for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i)
            if(s->frame.pressed==hits[i])flags[i]|=DFCS_PUSHED;
    }
    HICON icon=!s->frame.host.instance.detached && !IsRectEmpty(&s->caption.icon)?
        style_window_caption_icon(owner):NULL;
    if(!s->frame.host.instance.detached)
        win2k_caption_render_with_colors(dc,&s->caption,title,icon,s->font,s->dpi,
            win2k_caption_theme_palette(s->frame.active,s->themed?&s->theme:NULL),colors,s->maximized,flags);
    RestoreDC(dc,saved);
}

static LRESULT hit(void *context,POINT point)
{
    modern_frame *s=context;
    if(PtInRect(&s->client,point))return HTCLIENT;
    if(!PtInRect(&s->bounds,point))return HTNOWHERE;
    for(unsigned i=0;i<ARRAYSIZE(s->caption.buttons);++i)
        if(PtInRect(&s->caption.buttons[i],point))return hits[i];
    if(PtInRect(&s->caption.icon,point))return HTSYSMENU;
    if(PtInRect(&s->caption.band,point))return HTCAPTION;
    if(s->sizeable && point.y<s->caption.band.top && point.x>=s->client.left && point.x<s->client.right)
        return HTTOP;
    SIZE corner={s->caption.band.left+MulDiv(16,(int)s->dpi,96),s->client.top};
    return style_uwp_resize_margin_hit(HTBORDER,point,&s->bounds,&s->client,corner,s->sizeable);
}

static BOOL button(void *context,int part)
{
    if(part==HTSYSMENU || part==HTHELP || part==HTMINBUTTON || part==HTMAXBUTTON)return TRUE;
    if(part!=HTCLOSE)return FALSE;
    modern_frame *s=context;
    UINT state=GetMenuState(GetSystemMenu(s->frame.host.owner,FALSE),SC_CLOSE,MF_BYCOMMAND);
    return state!=(UINT)-1 && !(state&(MF_GRAYED|MF_DISABLED));
}

static void invoke(void *context,int part,UINT message)
{
    modern_frame *s=context;
    HWND owner=s->frame.host.owner;
    UINT command=0;
    if(message==WM_LBUTTONUP){
        if(part==HTMINBUTTON)command=SC_MINIMIZE;
        else if(part==HTMAXBUTTON)command=IsZoomed(owner)?SC_RESTORE:SC_MAXIMIZE;
        else if(part==HTCLOSE)command=SC_CLOSE;
        else if(part==HTHELP)command=SC_CONTEXTHELP;
        else if(part==HTSYSMENU)command=SC_KEYMENU;
    }else if(message==WM_LBUTTONDBLCLK){
        if(part==HTSYSMENU && button(s,HTCLOSE))command=SC_CLOSE;
        else if(part==HTCAPTION && !IsRectEmpty(&s->caption.buttons[WIN2K_MAXIMIZE]))
            command=IsZoomed(owner)?SC_RESTORE:SC_MAXIMIZE;
    }else if(message==WM_RBUTTONUP && (part==HTCAPTION || part==HTSYSMENU))command=SC_KEYMENU;
    if(command)PostMessageW(owner,WM_SYSCOMMAND,command,command==SC_KEYMENU?L' ':0);
}

static BOOL menu(void *context,HWND window,UINT message,WPARAM a,LPARAM b,BOOL owner,LRESULT *result)
{
    (void)owner;
    modern_frame *s=context;
    if(message==WM_INITMENUPOPUP && s->frame.tracking && (HMENU)a==GetSystemMenu(s->frame.host.owner,FALSE))
        b=MAKELPARAM(LOWORD(b),TRUE);
    return style_menu_owner_message(&s->frame.menu,window,message,a,b,&win2k_system_menu_operations,
        s->themed?&s->theme:NULL,result);
}

static void dispose(void *context)
{
    modern_frame *s=context;
    if(s->font)DeleteObject(s->font);
    free(s);
}

static const style_uwp_frame_ops ops={geometry,paint,hit,button,invoke,menu,dispose};

BOOL win2k_uwp_frame_uses(HWND owner)
{return style_uwp_frame_uses(owner,&ops);}

BOOL win2k_uwp_frame_apply(HWND owner,HWND title,const style_uwp_frame_action *actions,unsigned count,
    const classic_theme *theme)
{
    if(!title){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!style_uwp_frame_validate(owner,title,actions,count))return FALSE;
    modern_frame *s=calloc(1,sizeof(*s)+(size_t)count*sizeof(*s->actions));
    if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->themed=theme!=NULL;
    if(theme)s->theme=*theme;
    return style_uwp_frame_start(&s->frame,owner,title,&ops,s,actions,count,s->actions);
}
