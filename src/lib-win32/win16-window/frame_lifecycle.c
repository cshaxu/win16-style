#include "frame_lifecycle.h"
#include "frame_layout.h"
#include "frame_dispatch.h"
#include "../win16-common/font.h"
#include <dwmapi.h>

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
    s->bar_selected=-1;s->creation_owner=owner;s->calls=1;
    s->pending_frame=creating && normal && !(style&WS_VISIBLE);
    {WCHAR name[32];s->dialog=GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"#32770");}
    if(p)s->palette=*p;else win16_style_default_palette(&s->palette);
    s->active=(ex&WS_EX_MDICHILD)?
        (HWND)SendMessageW(GetParent(w),WM_MDIGETACTIVE,0,0)==w:GetActiveWindow()==w;
    if(!v->refresh_font(w,s)){win16_control_frame_state_dispose(s);SetLastError(ERROR_INVALID_DATA);return FALSE;}
    {win16_style_layout checked;
        if(!v->layout(w,s,&checked)){win16_control_frame_state_dispose(s);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}}
    win16_control_client_screen(w,&client);
    /* POLICY is set-only. Preserve the observable rendering state instead;
     * Windows does not expose whether it came from USEWINDOWSTYLE or ENABLED. */
    BOOL nc_enabled=FALSE;
    s->policy_saved=!(style&WS_CHILD) && SUCCEEDED(v->get_window_attribute(w,DWMWA_NCRENDERING_ENABLED,
        &nc_enabled,sizeof(nc_enabled)));
    s->old_policy=nc_enabled?DWMNCRP_ENABLED:DWMNCRP_DISABLED;
    if(cancelled && *cancelled){win16_control_frame_state_dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
    if(!SetWindowSubclass(w,v->style_proc,(UINT_PTR)v->style_proc,(DWORD_PTR)s)){win16_control_frame_state_dispose(s);return FALSE;}
    /* Bars may be added by range/visibility changes after attachment. Keep
     * one frame tracker ready instead of conditionally installing it later. */
    if(!v->scrollbars_install(w,&s->palette,s,&s->detached)){
        DWORD error=GetLastError();
        if(v->state(w)==s)v->remove(w);
        if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
        SetLastError(error);return FALSE;
    }
    /* Initial minimized/maximized creation is valid too. Native USER owns
     * its saved normal placement; never resize that state to preserve the
     * transient icon/maximized client rectangle. Restore uses normal layout. */
    if(!s->pending_frame)v->activate_frame(w,s,&client,normal);
    --s->calls;
    if(s->detached){win16_control_frame_state_dispose(s);SetLastError(cancelled && *cancelled?ERROR_CANCELLED:ERROR_INVALID_WINDOW_HANDLE);return FALSE;}
    return TRUE;
}
BOOL win16_control_menu_owner_install(HWND w,const void *owner,const BOOL *cancelled,const win16_control_frame_lifecycle_services *v)
{
 if(!v->local_window(w)||v->state(w)){SetLastError(ERROR_INVALID_STATE);return FALSE;}
 style_window *s=win16_control_frame_state_create();if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
 s->creation_owner=owner;s->menu_only=TRUE;s->dpi=GetDpiForWindow(w);if(!s->dpi)s->dpi=96;
 win16_style_default_palette(&s->palette);s->menu_font=win16_style_menu_font_acquire(s->dpi);
 if(!s->menu_font || (cancelled&&*cancelled)){win16_control_frame_state_dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
 if(!SetWindowSubclass(w,v->style_proc,(UINT_PTR)v->style_proc,(DWORD_PTR)s)){DWORD error=GetLastError();win16_control_frame_state_dispose(s);SetLastError(error);return FALSE;}
 return TRUE;
}
BOOL win16_control_frame_remove_owned(HWND w,const void *owner,const win16_control_frame_lifecycle_services *v)
{
    style_window *s=v->state(w);
    if(!owner || !s || s->creation_owner!=owner)return FALSE;
    return v->remove(w);
}

BOOL win16_control_frame_remove(HWND w,const win16_control_frame_lifecycle_services *v)
{
    style_window *s;RECT client;BOOL normal;
    if(!v->local_window(w))return FALSE;
    s=v->state(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    if(s->menu_only){if(!RemoveWindowSubclass(w,v->style_proc,(UINT_PTR)v->style_proc))return FALSE;++s->calls;s->detached=TRUE;while(s->menus)win16_control_frame_state_end_menu(s);win16_style_menu_restore(&s->menu_items);if(!--s->calls)win16_control_frame_state_dispose(s);return TRUE;}
    normal=!IsIconic(w)&&!IsZoomed(w);win16_control_client_screen(w,&client);
    if(!RemoveWindowSubclass(w,v->style_proc,(UINT_PTR)v->style_proc))return FALSE;
    ++s->calls;
    win16_control_frame_cancel_repaint(s);
    s->detached=TRUE;
    v->control_remove(w,s);
    while(s->menus)win16_control_frame_state_end_menu(s);
    win16_style_menu_restore(&s->menu_items);
    if(s->bar_keyboard)v->leave_bar(w,s,TRUE);
    if(s->pressed && GetCapture()==w)ReleaseCapture();
    win16_control_frame_restore(w,&s->pending_frame,&s->policy_saved,&s->old_policy,&client,normal&&!s->keep_outer,v->set_window_attribute);
    if(!--s->calls)win16_control_frame_state_dispose(s);
    return TRUE;
}

BOOL win16_control_frame_set_palette(HWND w,const win16_style_palette *p,const win16_control_frame_lifecycle_services *v)
{
    style_window *s;
    if(!p){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!v->local_window(w))return FALSE;
    s=v->state(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    ++s->calls;s->palette=*p;v->scrollbars_palette(w,p,s);v->paint(w,s,NULL);
    if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
    return TRUE;
}
BOOL win16_control_frame_terminal(HWND w,const win16_control_frame_lifecycle_services *v)
{
    style_window *s;
    s=v->state(w);if(!s)return TRUE;
    if(!RemoveWindowSubclass(w,v->style_proc,(UINT_PTR)v->style_proc)){
        SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    ++s->calls;win16_control_frame_cancel_repaint(s);s->detached=TRUE;
    while(s->menus)win16_control_frame_state_end_menu(s);
    if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
    return TRUE;
}
