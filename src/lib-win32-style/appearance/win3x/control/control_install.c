#include "control_install.h"
#include "lib-win32-style/application/lifecycle/install.h"
#include "control_state.h"
#include "control_guard.h"
#include "control_class.h"
#include "fileowner_message.h"
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
    return !native_edit || style_control_combo_bind(&s->combo,&s->instance,&s->state,window,child,install->services->combo);
}
static BOOL setup_control(HWND w,void *context)
{
    control_install *install=context;control_style *s=install->state;
    const win16_control_install_services *services=install->services;
    BOOL frame_bars=install->frame_bars,result=TRUE;
    if(!frame_bars && style_control_is_combo(w) && !s->file_owner){
        const style_control_combo_setup_ops children={install,services->get_combo_info,install_combo_list,
            services->remove_control,setup_combo_edit};
        if(!style_control_combo_setup(w,&s->instance,&s->combo,services->combo,&children)){
            result=FALSE;goto installed;}
    }
    if(!frame_bars){if(s->combo_passthrough && ((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_DROPDOWNLIST){HFONT replacement=win16_style_system_acquire(s->state.original_font);s->state.font=replacement?replacement:s->state.original_font;}
        if(!s->combo_passthrough)SendMessageW(w,WM_SETFONT,(WPARAM)s->state.original_font,FALSE);
        if(!s->instance.detached && style_control_is_separator(w))SetWindowPos(w,NULL,0,0,0,0,
            SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        if(!s->instance.detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);}
installed:
    if(s->instance.detached){result=FALSE;SetLastError(ERROR_CANCELLED);}
    return result;
}

BOOL win16_control_install(HWND w,const win16_style_palette *palette,BOOL edit,BOOL original_border,BOOL frame_bars,
    int tab_count,const int *tabs,const void *creation_owner,const BOOL *cancelled,HWND common_dialog_combo_pending,
    const win16_control_install_services *services)
{
    DWORD_PTR existing;control_style *s;BOOL tabbed=tab_count>=0;LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(GetWindowSubclass(w,services->control_proc,(UINT_PTR)services->control_proc,&existing)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(tabbed){
        if(!style_control_list_class(w) || !(style&LBS_USETABSTOPS) || (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))){
            SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
        if((tab_count && !tabs) || (size_t)tab_count>((size_t)-1)/sizeof(int)){
            SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    }else if(!frame_bars && (!win16_control_supported(w,common_dialog_combo_pending) || style_control_is_edit(w)!=edit)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    s=win16_control_state_create();if(!s)return FALSE;
    s->instance.owner=creation_owner;
    s->header_pressed=-1;
    if(!style_control_state_prepare(w,&s->state,tabbed?tab_count:0,tabs)){
        DWORD error=GetLastError();win16_control_state_dispose(s);SetLastError(error);return FALSE;}
    s->edit_border=edit && original_border;
    s->scroll.resources.frame=frame_bars;
    s->common_dialog_combo=!frame_bars && w==common_dialog_combo_pending && style_control_dialog_combo_valid(w,GetParent(w),style_control_is_combo);
    s->common_dialog_parent=s->common_dialog_combo?GetParent(w):NULL;
    s->file_owner=!frame_bars && (ntw16s_vb_owner_file(w) || s->common_dialog_combo);s->combo_passthrough=services->combo_candidate(w);
    if(!frame_bars){s->state.original_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        if(cancelled && *cancelled){win16_control_state_dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
        /* The font query may reenter installation. Do not replace that
         * successful attachment's subclass data with this outer attempt. */
        if(GetWindowSubclass(w,services->control_proc,(UINT_PTR)services->control_proc,&existing)){
            win16_control_state_dispose(s);SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
        s->state.original_font=win16_style_system_source(s->state.original_font);}
    if(palette)s->palette=*palette;else win16_style_default_palette(&s->palette);
    control_install install={s,services,palette,frame_bars};
    const style_instance_install_ops ops={&install,cancelled,setup_control,NULL};
    return style_instance_install(&s->instance,w,services->control_proc,creation_owner,&ops);
}

BOOL win16_control_dialog_combo_install(HWND w,const void *owner,HWND *pending,const win16_control_dialog_combo_services *v)
{
    HWND previous=*pending;
    if(!style_control_dialog_combo_valid(w,GetParent(w),style_control_is_combo)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    *pending=w;
    BOOL result=v->install(w,NULL,FALSE,FALSE,FALSE,-1,NULL,owner,NULL);
    *pending=previous;
    if(result && IsWindow(w)){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(v->get_combo_info(w,&info)){
            if(info.hwndList && v->grid_install(info.hwndList,NULL,owner,NULL))
                (void)v->list_enable(info.hwndList);
            if(info.hwndItem && info.hwndItem!=w && v->is_edit(info.hwndItem))
                (void)v->control_install(info.hwndItem,TRUE,
                    (GetWindowLongPtrW(info.hwndItem,GWL_STYLE)&WS_BORDER)!=0,FALSE,NULL,owner,NULL);
        }
    }
    return result;
}
