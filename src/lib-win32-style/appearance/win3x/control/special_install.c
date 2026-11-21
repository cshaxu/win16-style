#include "lib-win32-style/application/lifecycle/install.h"
#include "special_install.h"
#include "control_class.h"
#include "../../../application/control/control_protocol.h"
#include "trackbar_paint.h"
#include "progress_paint.h"


static control_style *install_frame(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    if(!v->frame_install(w,palette,owner,cancelled))return NULL;
    style_instance *instance=style_instance_lookup(w,v->control_proc);
    if(!instance || instance->owner!=owner){SetLastError(ERROR_CANCELLED);return NULL;}
    return (control_style *)instance;
}
BOOL win16_control_special_updown_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    HMODULE native=style_control_protocol_updown_module(w);
    if(!native){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    control_style *s=install_frame(w,palette,owner,cancelled,v);if(!s)return FALSE;
    if(s->controls.updown_mode==CLASSIC_UPDOWN_RECOLOR)return TRUE;
    if(!style_control_updown_api_enabled()){
        style_instance_retain(&s->instance);
        SetLastError(ERROR_NOT_SUPPORTED);
        return style_instance_setup_finish(w,&s->instance,FALSE,owner,v->remove_owned);
    }
    s->updown=(style_control_updown_art){.art=&s->scroll_art,.metrics=s->controls.updown};
    win16_control_updown_colors(s);
    style_control_updown_bind_art(&s->bindings.updown,w,native,&s->updown,&s->instance,
        style_instance_acquire_owned,style_instance_return_owned);
    return TRUE;
}
BOOL win16_control_special_trackbar_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    if(!win16_style_trackbar_supported(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    control_style *s=install_frame(w,palette,owner,cancelled,v);if(!s)return FALSE;
    style_instance_retain(&s->instance);
    BOOL okay=style_control_notify_bind(w,&s->bindings.track,v->trackbar_notify,s);
    /* Native trackbars cache part drawing. Invalidate only this new control's
     * native cache after binding, without changing theme/global properties. */
    if(okay)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    return style_instance_setup_finish(w,&s->instance,okay,owner,v->remove_owned);
}
BOOL win16_control_special_toolbar_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    if(!style_control_protocol_toolbar(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    control_style *s=install_frame(w,palette,owner,cancelled,v);if(!s)return FALSE;
    style_instance_retain(&s->instance);
    BOOL okay=style_control_notify_bind(w,&s->bindings.toolbar,v->toolbar_notify,s);
    return style_instance_setup_finish(w,&s->instance,okay,owner,v->remove_owned);
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
    if(!style_control_protocol_legacy_progress(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    control_style *s=install_frame(w,palette,owner,cancelled,v);if(!s)return FALSE;
    s->legacy_progress_border=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    return TRUE;
}
BOOL win16_control_special_tree_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    if(!style_control_protocol_is(w,WC_TREEVIEWW)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    control_style *s=install_frame(w,palette,owner,cancelled,v);if(!s)return FALSE;
    s->bindings.scroll.resources.native_border=TRUE;
    return TRUE;
}
BOOL win16_control_special_grid_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    control_style *s=install_frame(w,palette,owner,cancelled,v);if(!s)return FALSE;
    style_instance_retain(&s->instance);s->bindings.scroll.resources.native_border=TRUE;
    /* Owner-drawn lists use the shared frame scrollbar painter but may not
     * have the ordinary control attachment's theme lease. Common-controls
     * theme animations can otherwise repaint over that scrollbar later. */
    const style_control_list_theme_ops theme={s,cancelled,v->theme_retained,v->theme_capture,v->theme_enable};
    const style_control_setup_options options={.frame_bars=TRUE,.list_theme=&theme};
    BOOL result=style_control_bindings_setup(w,&s->instance,&s->state,&s->bindings,&options);
    return style_instance_setup_finish(w,&s->instance,result,owner,v->remove_owned);
}
BOOL win16_control_special_picture_install(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled,const win16_control_special_services *v)
{
    if(!ntw16s_vb_picture(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    control_style *s=install_frame(w,palette,owner,cancelled,v);if(!s)return FALSE;
    s->bindings.scroll.resources.native_border=TRUE;
    return TRUE;
}
