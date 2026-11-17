#include "control_install.h"
#include "lib-win32-style/application/lifecycle/install.h"
#include "control_state.h"
#include "control_guard.h"
#include "control_class.h"
#include "../common/font.h"


typedef struct control_install {
    control_style *state;
    const win16_control_install_services *services;
    const win16_style_palette *palette;
    BOOL frame_bars;
} control_install;
static BOOL install_combo_list(HWND window,void *context)
{
    const control_install *install=context;
    return install->services->install_control(window,install->palette);
}
static BOOL setup_combo_edit(HWND window,HWND child,void *context)
{
    const control_install *install=context;control_style *s=install->state;
    BOOL native_edit=style_control_vb_combo(window) && child!=window && style_control_combo_edit(child,style_control_vb_combo);
    if(s->instance.detached)return TRUE;
    return !native_edit || style_control_combo_bind(&s->bindings.combo,&s->instance,&s->state,window,child,install->services->combo);
}
static BOOL setup_control(HWND w,void *context)
{
    control_install *install=context;control_style *s=install->state;
    const win16_control_install_services *services=install->services;
    const style_control_combo_setup_ops children={install,services->get_combo_info,install_combo_list,services->remove_control,setup_combo_edit};
    const style_control_setup_options options={install->frame_bars,s->file_owner,s->combo_passthrough,services->combo,&children,NULL};
    return style_control_bindings_setup(w,&s->instance,&s->state,&s->bindings,&options);
}

BOOL win16_control_install(HWND w,const win16_style_palette *palette,BOOL edit,BOOL original_border,BOOL frame_bars,
    int tab_count,const int *tabs,const void *creation_owner,const BOOL *cancelled,HWND common_dialog_combo_pending,
    const win16_control_install_services *services)
{
    control_style *s;BOOL tabbed=tab_count>=0;LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(!style_instance_preflight(w,services->control_proc))return FALSE;
    if(tabbed){
        if(!style_control_list_class(w) || !(style&LBS_USETABSTOPS) || (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))){
            SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
        if((tab_count && !tabs) || (size_t)tab_count>((size_t)-1)/sizeof(int)){
            SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    }else if(!frame_bars && (!win16_control_supported(w,common_dialog_combo_pending) || style_control_is_edit(w)!=edit)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    s=win16_control_state_create();if(!s)return FALSE;
    s->instance.owner=creation_owner;
    if(!style_control_state_prepare(w,&s->state,tabbed?tab_count:0,tabs)){
        DWORD error=GetLastError();win16_control_state_dispose(s);SetLastError(error);return FALSE;}
    s->edit_border=edit && original_border;
    s->bindings.scroll.resources.frame=frame_bars;
    s->common_dialog_combo=!frame_bars && w==common_dialog_combo_pending && style_control_dialog_combo_valid(w,GetParent(w),style_control_is_combo);
    s->common_dialog_parent=s->common_dialog_combo?GetParent(w):NULL;
    s->file_owner=!frame_bars && (ntw16s_vb_owner_file(w) || s->common_dialog_combo);s->combo_passthrough=services->combo_candidate(w);
    if(!frame_bars && !style_control_font_read(w,&s->state,services->control_proc,cancelled)){
        DWORD error=GetLastError();win16_control_state_dispose(s);SetLastError(error);return FALSE;}
    if(palette)s->palette=*palette;else win16_style_default_palette(&s->palette);
    control_install install={s,services,palette,frame_bars};
    const style_instance_install_ops ops={&install,cancelled,setup_control,NULL};
    return style_instance_install(&s->instance,w,services->control_proc,creation_owner,&ops);
}
