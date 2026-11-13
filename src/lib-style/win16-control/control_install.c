#include "control_install.h"
#include "control_state.h"
#include "control_guard.h"
#include "control_class.h"
#include "fileowner_message.h"
#include "../win16-common/font.h"
#include <stdlib.h>


BOOL win16_control_install(HWND w,const win16_style_palette *palette,BOOL edit,BOOL original_border,BOOL frame_bars,
    int tab_count,const int *tabs,const void *creation_owner,const BOOL *cancelled,HWND common_dialog_combo_pending,
    const win16_control_install_services *services)
{
    DWORD_PTR existing;control_style *s;BOOL result=TRUE,tabbed=tab_count>=0;LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(GetWindowSubclass(w,services->control_proc,(UINT_PTR)services->control_proc,&existing)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(tabbed){
        if(!style_control_list_class(w) || !(style&LBS_USETABSTOPS) || (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))){
            SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
        if((tab_count && !tabs) || (size_t)tab_count>((size_t)-1)/sizeof(int)){
            SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    }else if(!frame_bars && (!win16_control_supported(w,common_dialog_combo_pending) || style_control_is_edit(w)!=edit)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    s=win16_control_state_create();if(!s)return FALSE;
    s->creation_owner=creation_owner;
    s->header_pressed=-1;
    SetLastError(ERROR_SUCCESS);
    s->protocol=style_control_protocol_acquire(w);
    if(!s->protocol && GetLastError()==ERROR_ARITHMETIC_OVERFLOW){win16_control_state_dispose(s);SetLastError(ERROR_ARITHMETIC_OVERFLOW);return FALSE;}
    if(tabbed && tab_count){s->tab_positions=malloc((size_t)tab_count*sizeof(int));
        if(!s->tab_positions){win16_control_state_dispose(s);return FALSE;}
        CopyMemory(s->tab_positions,tabs,(size_t)tab_count*sizeof(int));s->tab_count=tab_count;}
    s->edit_border=edit && original_border;
    s->frame_scrollbars=frame_bars;
    s->common_dialog_combo=!frame_bars && w==common_dialog_combo_pending && style_control_dialog_combo_valid(w,GetParent(w),style_control_is_combo);
    s->common_dialog_parent=s->common_dialog_combo?GetParent(w):NULL;
    s->file_owner=!frame_bars && (ntw16s_vb_owner_file(w) || s->common_dialog_combo);s->combo_passthrough=services->combo_candidate(w);
    if(!frame_bars){s->original_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        if(cancelled && *cancelled){win16_control_state_dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
        /* The font query may reenter installation. Do not replace that
         * successful attachment's subclass data with this outer attempt. */
        if(GetWindowSubclass(w,services->control_proc,(UINT_PTR)services->control_proc,&existing)){
            win16_control_state_dispose(s);SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
        s->original_font=win16_style_system_source(s->original_font);}
    if(palette)s->palette=*palette;else win16_style_default_palette(&s->palette);
    if(!SetWindowSubclass(w,services->control_proc,(UINT_PTR)services->control_proc,(DWORD_PTR)s)){win16_control_state_dispose(s);return FALSE;}
    /* Child attachment and WM_SETFONT can synchronously remove/destroy us.
     * Retain this attempt until its result is known, just like a callback. */
    ++s->calls;
    if(!frame_bars && style_control_is_combo(w) && !s->file_owner){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(!services->get_combo_info(w,&info) || !services->install_control(info.hwndList,palette)){
            if(!s->detached)services->remove_control(w);
            result=FALSE;
            goto installed;
            }
        if(s->detached){services->remove_control(info.hwndList);goto installed;}
        s->combo_list=info.hwndList;
        if(GetWindowSubclass(info.hwndList,services->control_proc,(UINT_PTR)services->control_proc,&existing))
            ((control_style *)existing)->owner=s;
        BOOL native_edit=style_control_vb_combo(w) && info.hwndItem!=w && style_control_combo_edit(info.hwndItem,style_control_vb_combo);
        if(s->detached)goto installed;
        if(native_edit){
            HFONT source=(HFONT)SendMessageW(info.hwndItem,WM_GETFONT,0,0);
            if(s->detached)goto installed;
            s->combo_parent=w;s->combo_edit_source=win16_style_system_source(source);
            DWORD_PTR child_owner;
            if(GetWindowSubclass(info.hwndItem,services->combo_proc,(UINT_PTR)services->combo_proc,&child_owner)){
                services->remove_control(w);SetLastError(ERROR_ALREADY_EXISTS);result=FALSE;goto installed;
            }
            if(!SetWindowSubclass(info.hwndItem,services->combo_proc,(UINT_PTR)services->combo_proc,(DWORD_PTR)s)){
                DWORD error=GetLastError();services->remove_control(w);SetLastError(error);result=FALSE;goto installed;
            }
            s->combo_edit=info.hwndItem;s->combo_bound=TRUE;++s->calls;
            SendMessageW(info.hwndItem,WM_SETFONT,(WPARAM)s->combo_edit_source,FALSE);
            if(s->detached)goto installed;
        }
    }
    if(!frame_bars){if(s->combo_passthrough && ((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_DROPDOWNLIST){HFONT replacement=win16_style_system_acquire(s->original_font);s->font=replacement?replacement:s->original_font;}
        if(!s->combo_passthrough)SendMessageW(w,WM_SETFONT,(WPARAM)s->original_font,FALSE);
        if(!s->detached && style_control_is_separator(w))SetWindowPos(w,NULL,0,0,0,0,
            SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        if(!s->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);}
installed:
    if(s->detached){result=FALSE;SetLastError(ERROR_CANCELLED);}
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return result;
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
