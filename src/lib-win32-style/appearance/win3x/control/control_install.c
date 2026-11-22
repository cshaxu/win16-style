#include "control_install.h"
#include "lib-win32-style/application/lifecycle/install.h"
#include "control_state.h"
#include "control_guard.h"
#include "control_class.h"
#include <string.h>

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
    BOOL native_edit=s->controls.control_font_points && style_control_vb_combo(window) && child!=window && style_control_combo_edit(child,style_control_vb_combo);
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
    style_control_theme source={0};
    DWORD theme_error=services->load_theme(&source);
    if(theme_error){win16_control_state_dispose(s);SetLastError(theme_error);return FALSE;}
    BOOL have_theme=source.resources!=NULL;
    const style_control_theme *theme=&source;
    const unsigned native=have_theme?classic_control_native_roles(&theme->controls):0;
    const unsigned roles=frame_bars?native&(STYLE_ROLE_UPDOWN|STYLE_ROLE_TRACKBAR):native;
    if(((native&STYLE_ROLE_PROGRESS_BORDER) && frame_bars && style_control_protocol_legacy_progress(w)) ||
       ((native&STYLE_ROLE_EDIT) && !frame_bars && edit) ||
       style_control_role_native(w,roles,style_control_class_is)){
        if(source.resources)FreeLibrary(source.resources);
        win16_control_state_dispose(s);
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }
    if(have_theme){
        s->scroll_metrics=theme->scroll_metrics;
        s->scroll_art=theme->scroll_art;
        s->combo_arrow=theme->combo_arrow;
        s->choice_colors=theme->choice;
        if(classic_choice_has_art(&s->choice_colors)){
            s->choice_module=source.resources;
            s->read_choice_art=source.read_choice_art;
            source.resources=NULL;
        }
    }
    s->scroll_policy=(style_scroll_policy){.drag=theme->scroll_metrics.drag,.hit=style_scroll_hit};
    style_scroll_repeat_configure(&s->scroll_policy,theme->scroll_metrics.initial_repeat_ms,theme->scroll_metrics.repeat_ms);
    s->controls=theme->controls;
    s->state.font_recipe=s->controls.control_font_points;
    s->themed=!palette && have_theme;
    if(palette)s->palette=*palette;
    else if(!s->themed)style_window_palette_default(&s->palette);
    if(s->themed){
        s->palette=theme->window_palette;
        s->push_colors=theme->push;
        s->label_colors=theme->label;
        memcpy(s->edit_colors,theme->edit,sizeof(s->edit_colors));
        s->list_colors=theme->list;
        s->combo_colors=theme->combo;
        s->tab_colors=theme->tab;
        s->header_colors=theme->header;
        memcpy(s->progress_colors,theme->progress,sizeof(s->progress_colors));
        memcpy(s->trackbar_colors,theme->trackbar,sizeof(s->trackbar_colors));
        s->group_colors=theme->group;
        s->frame_colors=theme->frame;
        s->toolbar_colors=theme->bars[CLASSIC_BAR_TOOLBAR];
        s->scroll_arrow_colors=theme->scroll_arrow;
        s->scroll_colors=theme->scroll;
    }
    if(source.resources)FreeLibrary(source.resources);
    control_install install={s,services,palette,frame_bars};
    const style_instance_install_ops ops={&install,cancelled,setup_control,NULL};
    return style_instance_install(&s->instance,w,services->control_proc,creation_owner,&ops);
}
