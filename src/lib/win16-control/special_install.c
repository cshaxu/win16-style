#include "special_install.h"
#include "control_class.h"
#include "control_protocol.h"
#include "scroll_assets.h"
#include "trackbar_paint.h"
#include "progress_paint.h"

static BOOL is_list(HWND w){WCHAR name[16];return ntw16s_class_is(w,L"LISTBOX") || (GetClassNameW(w,name,16) && !lstrcmpiW(name,L"ComboLBox"));}

BOOL win16_control_special_updown_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    DWORD_PTR value;HMODULE native=ntw16s_control_protocol_updown_module(w);
    if(!native || !ntw16s_updown_api_enabled()){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!v->frame_install(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;
    ntw16s_updown_register(&s->updown,w,native,&s->palette,win16_control_scroll_glyphs(),s,v->updown_acquire,v->updown_release);
    return TRUE;
}
BOOL win16_control_special_trackbar_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    DWORD_PTR value;
    if(!win16_style_trackbar_supported(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!v->frame_install(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;++s->calls;s->track_window=w;s->track_parent=GetParent(w);
    BOOL okay=s->track_parent && SetWindowSubclass(s->track_parent,v->trackbar_notify,(UINT_PTR)w,(DWORD_PTR)s);
    /* Native trackbars cache part drawing. Invalidate only this new control's
     * native cache after binding, without changing theme/global properties. */
    if(okay)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    if(s->detached)okay=FALSE;
    if(!okay && !s->detached)v->remove_owned(w,owner);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return okay;
}
BOOL win16_control_special_toolbar_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    DWORD_PTR value;
    if(!ntw16s_control_protocol_toolbar(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!v->frame_install(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;++s->calls;s->toolbar_window=w;s->toolbar_parent=GetParent(w);
    BOOL okay=s->toolbar_parent && SetWindowSubclass(s->toolbar_parent,v->toolbar_notify,(UINT_PTR)w,(DWORD_PTR)s);
    if(s->detached)okay=FALSE;
    if(!okay && !s->detached)v->remove_owned(w,owner);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return okay;
}
BOOL win16_control_special_progress_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    if(!win16_style_progress_supported(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    /* No text or font property belongs to this painter. Reuse the owned
     * passive-font leaf; the progress branch owns its paint, not bar input. */
    return v->frame_install(w,palette,owner,cancelled);
}
BOOL win16_control_special_legacy_progress_border_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    DWORD_PTR data;
    if(!ntw16s_control_protocol_legacy_progress(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!v->frame_install(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->legacy_progress_border=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    return TRUE;
}
BOOL win16_control_special_tree_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    DWORD_PTR data;
    if(!ntw16s_control_protocol_is(w,WC_TREEVIEWW)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!v->frame_install(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->native_border=TRUE;
    return TRUE;
}
BOOL win16_control_special_grid_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    DWORD_PTR data;control_style *s;BOOL result=TRUE;
    if(!v->frame_install(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    s=(control_style *)data;++s->calls;s->native_border=TRUE;
    /* Owner-drawn lists use the shared frame scrollbar painter but may not
     * have the ordinary control attachment's theme lease. Common-controls
     * theme animations can otherwise repaint over that scrollbar later. */
    if(is_list(w) && !v->theme_retained(w)){
        if(!v->theme_capture(w))result=FALSE;
        else {s->list_theme_owned=TRUE;if(!v->theme_enable(w,s))result=FALSE;}
    }
    if(s->detached || (cancelled && *cancelled)){result=FALSE;SetLastError(ERROR_CANCELLED);}
    if(!result && !s->detached)v->remove_owned(w,owner);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return result;
}
BOOL win16_control_special_picture_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    DWORD_PTR data;
    if(!ntw16s_vb_picture(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!v->frame_install(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->native_border=TRUE;
    return TRUE;
}
