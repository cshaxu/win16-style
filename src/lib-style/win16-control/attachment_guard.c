#include "attachment_state.h"
#include "lib-style/common/control_scope.h"
#include "control_class.h"
#include "vcl_identity.h"
#include "lib-style/common/control_vcl_spin.h"
#include "choice_paint.h"

void win16_attachment_retire_frames(HWND w,attachment *a)
{
    if(a->controlbar_candidate && a->controlbar_parent && GetParent(w)!=a->controlbar_parent){a->controlbar_retired=TRUE;a->controlbar_verified=FALSE;}
    if(a->panel_frame_candidate && a->panel_frame_parent && GetParent(w)!=a->panel_frame_parent){a->panel_frame_retired=TRUE;a->panel_frame_verified=FALSE;}
    if(a->group_frame_candidate && a->group_frame_parent && GetParent(w)!=a->group_frame_parent){a->group_frame_retired=TRUE;a->group_frame_verified=FALSE;}
}
void win16_attachment_guard_cool(HWND w,UINT m,attachment *a,const win16_attachment_guard_services *v)
{
    if(a->cool_candidate && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !v->cool_identity(w,a))(void)v->remove(w);
}
void win16_attachment_guard_roles(HWND w,UINT m,attachment *a,const win16_attachment_guard_services *v)
{
    if(m!=WM_DESTROY && m!=WM_NCDESTROY){
        if(a->picture && !a->detached && !ntw16s_vb_picture(w))
            (void)v->remove(w);
        if(a->button_owner && !a->detached && !ntw16s_vb_graphical_button(w))
            (void)v->remove(w);
        if(a->file_owner && !a->detached &&
            style_control_vb_checklist_class(w) && !ntw16s_vb_checklist_owner(w))
            (void)v->remove(w);
        if((a->file_owner || a->button_owner || a->checklist) && !a->detached && a->report_parent && GetParent(w)!=a->report_parent)
            (void)v->remove(w);
        if(a->inplace_candidate && !a->detached && !win16_control_inplace_or_lookup(w,a->inplace_owner,a->inplace_parent))(void)v->remove(w);
        if(a->lookup_popup_candidate && !a->detached && (v->lookup(a->lookup_popup_source)!=a->lookup_popup_source_attachment || !win16_control_lookup_popup_identity(w,a->lookup_popup_source,a->lookup_popup_source_parent,a->lookup_popup_owner)))(void)v->remove(w);
        if(a->mdi_candidate && !a->detached && (v->lookup(a->mdi_host)!=a->mdi_host_attachment || !v->mdi_identity(w,a->mdi_parent,a->mdi_host,a->mdi_owner)))(void)v->remove(w);
        if(a->nav_candidate && !a->detached && !win16_control_nav_identity(w,a->nav_owner,a->nav_parent))(void)v->remove(w);
    }
    if(a->scroll_candidate && !a->detached && GetParent(w)!=a->scroll_parent)(void)v->remove(w);
    if(a->vcl_report_candidate && !a->detached && GetParent(w)!=a->vcl_report_parent)(void)v->remove(w);
}

void win16_attachment_post_box(HWND w,attachment *a,BOOL box_witnessed,const win16_attachment_post_services *v)
{
    if(box_witnessed && !a->box_installed && !a->detached && win16_control_box_identity(w,a->box_owner,a->box_parent)){
        DWORD saved=GetLastError();a->box_installed=TRUE;
        if(v->grid_install(w,NULL,a,&a->detached) && !a->detached && win16_control_box_identity(w,a->box_owner,a->box_parent))RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
        else if(!a->detached){a->box_installed=FALSE;v->remove_owned(w,a);}
        SetLastError(saved);
    }
}
void win16_attachment_post_controls(HWND w,UINT m,attachment *a,const win16_attachment_post_services *v)
{
    if(m!=WM_PAINT)return;
    /* Runtime controls may initially have a temporary parent. Bind the
     * verified border to the current parent only at the first real paint. */
    if(a->vb_data_candidate && !a->vb_data_attempted && !a->detached){
        DWORD saved=GetLastError();HWND parent=GetParent(w);
        if(win16_control_vb_data_role(w,parent)){
            a->vb_data_attempted=TRUE;a->vb_data_parent=parent;a->vb_data_edge=TRUE;
            BOOL installed=v->grid_install(w,NULL,a,&a->detached);
            if(installed && !a->detached && win16_control_vb_data_role(w,parent))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
            else if(!a->detached){a->vb_data_edge=FALSE;v->remove_owned(w,a);}
        }
        SetLastError(saved);
    }
    if(a->vb_user_candidate && !a->vb_user_attempted && !a->detached){
        DWORD saved=GetLastError();HWND parent=GetParent(w);
        if(win16_control_vb_user_edge_identity(w,parent)){
            a->vb_user_attempted=TRUE;
            a->vb_user_parent=parent;a->vb_user_edge=TRUE;
            BOOL installed=v->grid_install(w,NULL,a,&a->detached);
            if(installed && !a->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
            else if(!a->detached){a->vb_user_edge=FALSE;v->remove_owned(w,a);}
        }
        SetLastError(saved);
    }
    if(a->rich_candidate && !a->rich_installed && a->rich_witness && v->rich_identity(w,a)){
        DWORD saved=GetLastError();a->rich_installed=TRUE;
        if((a->rich_month || v->grid_install(w,NULL,a,&a->detached)) && !a->detached && v->rich_identity(w,a)){if(!a->rich_month)RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);}
        else if(!a->detached){a->rich_installed=FALSE;v->remove_owned(w,a);}
        SetLastError(saved);
    }
    if(a->grid_candidate && !a->grid_attempted && !a->detached){
        DWORD saved=GetLastError();SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_ALL};DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        a->grid_attempted=TRUE;
        if(win16_control_grid_identity(w,a->grid_owner,a->grid_parent) &&
            ((style_control_class_is(w,L"TValueListEditor") && !(style&(WS_HSCROLL|WS_VSCROLL))) ||
             (v->get_scroll_info(w,(style&WS_HSCROLL)?SB_HORZ:SB_VERT,&info) &&
              info.nMin<=info.nMax && info.nPos>=info.nMin && info.nPos<=info.nMax)) && !a->detached){
            BOOL installed=v->grid_install(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->grid_installed=TRUE;RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);}
            else if(!a->detached)v->remove_owned(w,a);
        }
        SetLastError(saved);
    }
}

static void release_failed_protocol(HWND w,attachment *a,BOOL (*remove_owned)(HWND,const void *))
{
    remove_owned(w,a);style_control_protocol_release(a->protocol);a->protocol=NULL;
}

void win16_attachment_post_toolbar(HWND w,UINT m,attachment *a,const win16_attachment_verified_services *v)
{
    if(m==WM_PAINT && a->vcl_toolbar_candidate && !a->vcl_toolbar_attempted && !a->detached && a->vcl_toolbar_witness){
        DWORD saved=GetLastError();a->vcl_toolbar_attempted=TRUE;
        style_control_protocol *protocol=win16_control_vcl_toolbar_identity(w,a->vcl_toolbar_owner,a->vcl_toolbar_parent)?style_control_protocol_bind_verified_toolbar(w,a->vcl_toolbar_witness):NULL;
        if(protocol && !a->detached){
            a->protocol=protocol;BOOL installed=v->toolbar_install(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->vcl_toolbar_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){release_failed_protocol(w,a,v->remove_owned);}
        }else if(protocol)style_control_protocol_release(protocol);
        SetLastError(saved);
    }
}
void win16_attachment_post_verified(HWND w,UINT m,attachment *a,const win16_attachment_verified_services *v)
{
    if(m!=WM_PAINT)return;
    if(a->group_button_candidate && !a->group_button_attempted && !a->detached && a->group_button_witness_owner){
        DWORD saved=GetLastError();a->group_button_attempted=TRUE;
        style_control_protocol *protocol=win16_control_group_button_identity(w)?style_control_protocol_bind_verified_group_button(w,a->group_button_witness_owner):NULL;
        if(protocol && !a->detached){
            a->protocol=protocol;
            BOOL installed=v->control_install(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached){a->group_button_parent=GetParent(w);a->group_button_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){release_failed_protocol(w,a,v->remove_owned);}
        }else if(protocol)style_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(a->scroll_candidate && !a->scroll_attempted && !a->detached && a->scroll_witness){
        DWORD saved=GetLastError();a->scroll_attempted=TRUE;
        style_control_protocol *protocol=GetParent(w)==a->scroll_parent?style_control_protocol_bind_verified_scroll(w,a->scroll_witness):NULL;
        if(protocol && !a->detached && GetParent(w)==a->scroll_parent){a->protocol=protocol;
            BOOL installed=v->control_install(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached && GetParent(w)==a->scroll_parent){a->scroll_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){release_failed_protocol(w,a,v->remove_owned);}
        }else if(protocol)style_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(a->edit_candidate && !a->edit_attempted && !a->detached && a->edit_witness_owner){
        DWORD saved=GetLastError();a->edit_attempted=TRUE;
        style_control_protocol *protocol=a->combo_candidate && !v->combo_same_role(w,a)?NULL:a->link_candidate?style_control_protocol_bind_verified_link(w,a->edit_witness_owner):a->combo_candidate?style_control_protocol_bind_verified_combo(w,a->edit_witness_owner):style_control_protocol_bind_verified_edit(w,a->edit_witness_owner);
        if(protocol && !a->detached && (!a->combo_candidate || v->combo_same_role(w,a))){
            a->protocol=protocol;
            BOOL installed=v->control_install(w,!a->link_candidate && !a->combo_candidate,(a->edit_original_style&WS_BORDER)!=0,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached && !a->link_candidate && !a->combo_candidate && !a->spinedit_candidate && (a->edit_original_style&ES_MULTILINE)){
                a->themed=w;
                installed=v->theme_enable(w,a) && v->edit_scrollbars_enable(w);
            }
            if(installed && !a->detached && (!a->combo_candidate || v->combo_same_role(w,a))){a->edit_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){v->remove_owned(w,a);if(a->themed){v->theme_remove(w,a);a->themed=NULL;}
                style_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)style_control_protocol_release(protocol);
        /* An empty link has no ideal height. Retry on a later paint only
         * after a fresh same-owner native SysLink forwarding witness. */
        if(a->link_candidate && !a->detached && !a->edit_installed){
            a->edit_attempted=FALSE;a->edit_witness_owner=NULL;
        }
        SetLastError(saved);
    }
    if(a->vcl_report_candidate && !a->vcl_report_attempted && !a->detached && a->vcl_report_witness)v->vcl_report_try_bind(w,a);
    if(a->tab_candidate && !a->tab_attempted && !a->detached && a->tab_witness_owner){
        DWORD saved=GetLastError();a->tab_attempted=TRUE;
        style_control_protocol *protocol=GetParent(w)==a->tab_parent?style_control_protocol_bind_verified_tab(w,a->tab_witness_owner):NULL;
        if(protocol && !a->detached){a->protocol=protocol;
            BOOL installed=v->control_install(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached){a->tab_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){release_failed_protocol(w,a,v->remove_owned);}
        }else if(protocol)style_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(a->track_candidate && !a->track_attempted && !a->detached && a->track_witness_owner){
        DWORD saved=GetLastError();a->track_attempted=TRUE;
        style_control_protocol *protocol=style_control_protocol_bind_verified_trackbar(w,a->track_witness_owner);
        if(protocol && !a->detached){
            a->protocol=protocol;
            BOOL installed=v->trackbar_install(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->track_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){release_failed_protocol(w,a,v->remove_owned);}
        }else if(protocol)style_control_protocol_release(protocol);
        SetLastError(saved);
    }
}

void win16_attachment_post_native(HWND w,UINT m,WPARAM p,attachment *a,HRGN print_clip,int print_clip_kind,BOOL color_update,const RECT *dirty,const win16_attachment_native_services *v)
{
    if(m==WM_PAINT && a->color_candidate && !a->color_attempted && !a->detached && a->color_witness_owner){
        DWORD saved=GetLastError();COMBOBOXINFO info={.cbSize=sizeof(info)};a->color_attempted=TRUE;
        if(v->color_identity(w,a,&info)){a->color_parent=GetParent(w);a->color_installed=TRUE;}
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->status_candidate && !a->status_attempted && !a->detached && a->status_witness_owner){
        DWORD saved=GetLastError();a->status_attempted=TRUE;
        if(v->status_identity(w,a,NULL)){a->status_parent=GetParent(w);a->status_installed=TRUE;a->status=TRUE;InvalidateRect(w,NULL,TRUE);}
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->date_candidate && !a->date_attempted && !a->detached && a->date_witness_owner){
        DWORD saved=GetLastError();DATETIMEPICKERINFO info={.cbSize=sizeof(info)};a->date_attempted=TRUE;
        if(v->date_identity(w,a,&info)){a->date_parent=GetParent(w);a->date_installed=TRUE;}
        SetLastError(saved);
    }
    if((a->color_installed || a->date_installed) && !a->detached && ((m==WM_PRINTCLIENT && print_clip_kind>=0) || (m==WM_PAINT && color_update))){
        DWORD saved=GetLastError();HDC dc=m==WM_PRINTCLIENT?(HDC)p:GetDC(w);
        if(dc){
            if(m==WM_PRINTCLIENT){int state=SaveDC(dc);if(state){
                if(SelectClipRgn(dc,print_clip_kind?print_clip:NULL)!=ERROR){
                    if(a->color_installed)v->color_button(w,a,dc,NULL);else v->date_button(w,a,dc,NULL);
                }
                RestoreDC(dc,state);
            }}else {if(a->color_installed)v->color_button(w,a,dc,dirty);else v->date_button(w,a,dc,dirty);ReleaseDC(w,dc);}
        }
        SetLastError(saved);
    }
    if(print_clip){DWORD saved=GetLastError();DeleteObject(print_clip);SetLastError(saved);}
}

void win16_attachment_native_prepare(HWND w,UINT m,WPARAM p,attachment *a,BOOL color,RECT *dirty,BOOL *update,HRGN *clip,int *clip_kind)
{
    *update=color && m==WM_PAINT && GetUpdateRect(w,dirty,FALSE);
    *clip=NULL;*clip_kind=-1;
    if((a->color_installed || a->date_installed) && !a->detached && m==WM_PRINTCLIENT && p){
        *clip=CreateRectRgn(0,0,0,0);
        if(*clip)*clip_kind=GetClipRgn((HDC)p,*clip);
    }
}

BOOL win16_attachment_install_style(HWND w,attachment *a,style_control_protocol *protocol,DWORD original_style,BOOL legacy_progress,BOOL tree,BOOL picture,BOOL control,BOOL edit,BOOL report,DWORD *out_error,const win16_attachment_install_services *v)
{
    BOOL installed;DWORD error;
    if(a->cool_candidate || a->box_candidate || a->controlbar_candidate || a->rich_candidate || a->vb_data_candidate || a->vb_user_candidate || a->checklist || a->status || a->spin || a->static_candidate || a->scroll_candidate || a->edit_candidate || a->group_button_candidate || a->grid_candidate || a->inplace_candidate || a->nav_candidate || a->vcl_toolbar_candidate || a->track_candidate || a->tab_candidate || a->vcl_report_candidate || a->color_candidate || a->date_candidate || a->status_candidate || a->bit_candidate || a->group_frame_candidate || a->panel_frame_candidate)installed=TRUE;
    else if(legacy_progress)installed=v->legacy_progress(w,NULL,a,&a->detached);
    else if(protocol && style_control_protocol_toolbar(w))
        installed=v->toolbar(w,NULL,a,&a->detached);
    else if(protocol && style_control_protocol_is(w,UPDOWN_CLASSW))installed=v->updown(w,NULL,a,&a->detached);
    else if(protocol && style_control_protocol_is(w,TRACKBAR_CLASSW))installed=v->trackbar(w,NULL,a,&a->detached);
    else if(protocol && style_control_protocol_is(w,PROGRESS_CLASSW))installed=v->progress(w,NULL,a,&a->detached);
    else if(tree)installed=v->tree(w,NULL,a,&a->detached);
    else if(picture)installed=v->picture(w,NULL,a,&a->detached);
    else if(a->lookup_popup_candidate)installed=v->grid(w,NULL,a,&a->detached);
    else if(a->button_owner)installed=v->frame_scrollbars(w,NULL,a,&a->detached);
    else if(control)installed=v->control(w,edit,(original_style&WS_BORDER)!=0,
        style_control_class_is(w,L"LISTBOX") && (original_style&LBS_USETABSTOPS) && !a->file_owner,NULL,a,&a->detached);
    else if(report && !(original_style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION)))installed=TRUE;
    else installed=v->frame(w,NULL,a,&a->detached);
    error=GetLastError();
    if(installed && !a->detached){
        if(control){
            HWND themed=NULL;BOOL list=FALSE;
            if(!a->file_owner && style_control_class_is(w,L"COMBOBOX")){
                COMBOBOXINFO info={.cbSize=sizeof(info)};
                if(v->get_combo_info(w,&info)){themed=info.hwndList;list=TRUE;}
            }else if(style_control_class_is(w,L"LISTBOX")){themed=w;list=TRUE;}
            else if(edit && (original_style&ES_MULTILINE))themed=w;
            if(themed){
                a->themed=themed;
                if(!v->theme_enable(themed,a))installed=FALSE;
                else {
                    installed=list?v->list_scrollbar_enable(themed):v->edit_scrollbar_enable(themed);}
                error=GetLastError();
            }
        }
    }
    if(installed && !a->detached && !a->vb_data_candidate && !a->scroll_candidate && !a->edit_candidate && !a->group_button_candidate && !a->grid_candidate && !a->vcl_toolbar_candidate && !a->track_candidate && !a->tab_candidate && !a->vcl_report_candidate && !a->color_candidate && !a->date_candidate && !a->status_candidate && !a->bit_candidate && a->themed!=w)v->theme_remove(w,NULL);
    if(a->detached){installed=FALSE;error=ERROR_CANCELLED;}
    if(!installed && !a->detached)v->remove(w);
    *out_error=error;return installed;
}

void win16_attachment_report_bind(HWND w,attachment *a,const win16_attachment_report_services *v)
{
    WCHAR name[64];
    if(!GetClassNameW(w,name,ARRAYSIZE(name)) ||
        (lstrcmpW(name,WC_LISTVIEWW) && lstrcmpW(name,L"TListView") &&
         lstrcmpW(name,WC_TREEVIEWW) && lstrcmpW(name,L"TTreeView") && lstrcmpW(name,L"TShellTreeView") &&
         lstrcmpW(name,L"TreeView20WndClass")))return;
    style_control_protocol *protocol=style_control_protocol_bind(w);
    if(!protocol)return;
    HMODULE module=style_control_protocol_list_module(w);
    BOOL tree=style_control_protocol_tree_module(w)!=NULL;if(tree)module=style_control_protocol_tree_module(w);
    HWND parent=GetParent(w);
    if(!a->detached && module && parent &&
        SetWindowSubclass(parent,v->notify,(UINT_PTR)w,(DWORD_PTR)a)){
        a->protocol=protocol;a->report_window=w;a->report_parent=parent;a->report_module=module;a->tree=tree;
    }else style_control_protocol_release(protocol);
}
/* The runtime-owner path is accepted only after this exact HWND forwards its
 * own paint to the registered native class. Keep the public protocol checks. */
void win16_attachment_vcl_report_bind(HWND w,attachment *a,const win16_attachment_report_services *v)
{
    DWORD saved=GetLastError();a->vcl_report_attempted=TRUE;
    BOOL tree=a->vcl_report_tree;
    style_control_protocol *protocol=GetParent(w)==a->vcl_report_parent?
        style_control_protocol_bind_verified_report(w,a->vcl_report_witness,tree):NULL;
    if(protocol && !a->detached){
        HMODULE module=tree?style_control_protocol_tree_module(w):style_control_protocol_list_module(w);
        HWND parent=GetParent(w);
        if(module && parent==a->vcl_report_parent && SetWindowSubclass(parent,v->notify,(UINT_PTR)w,(DWORD_PTR)a)){
            a->protocol=protocol;a->report_window=w;a->report_parent=parent;a->report_module=module;a->tree=tree;
            DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
            BOOL installed=tree?v->tree_install(w,NULL,a,&a->detached):
                !(style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION))?TRUE:v->frame_install(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->vcl_report_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){(void)v->remove(w);}
        }else style_control_protocol_release(protocol);
    }else if(protocol)style_control_protocol_release(protocol);
    if(!a->detached && !a->vcl_report_installed){a->vcl_report_attempted=FALSE;a->vcl_report_witness=NULL;}
    SetLastError(saved);
}
void win16_attachment_report_unbind(attachment *a,const win16_attachment_report_services *v)
{
    style_control_subclass_remove_owned(a->report_parent,v->notify,(UINT_PTR)a->report_window,a);
    a->report_parent=NULL;
}

BOOL win16_attachment_attached(HWND w,const win16_attachment_lifecycle_services *v)
{
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    attachment *a=v->lookup(w);return a && (!a->vcl_report_candidate || (a->vcl_report_installed && GetParent(w)==a->vcl_report_parent)) && (!a->cool_candidate || (a->cool_verified && v->cool_identity(w,a))) && (!a->mdi_candidate || (v->lookup(a->mdi_host)==a->mdi_host_attachment && v->mdi_identity(w,a->mdi_parent,a->mdi_host,a->mdi_owner))) && (!a->controlbar_candidate || (a->controlbar_verified && v->controlbar_identity(w,a))) && (!a->box_candidate || (a->box_installed && win16_control_box_identity(w,a->box_owner,a->box_parent))) && (!a->modern_spin || a->modern_spin_verified) && (!a->vb_data_candidate || a->vb_data_edge) && (!a->vb_user_candidate || v->vb_user_label_target(w)) && (!a->static_candidate || a->static_verified) && (!a->scroll_candidate || (a->scroll_installed && GetParent(w)==a->scroll_parent)) && (!a->edit_candidate || a->edit_installed) && (!a->group_button_candidate || a->group_button_installed) && (!a->grid_candidate || a->grid_installed) && (!a->vcl_toolbar_candidate || a->vcl_toolbar_installed) && (!a->track_candidate || a->track_installed) && (!a->tab_candidate || a->tab_installed) && (!a->color_candidate || a->color_installed) && (!a->date_candidate || a->date_installed) && (!a->status_candidate || a->status_installed) && (!a->rich_candidate || (a->rich_installed && v->rich_identity(w,a))) && (!a->bit_candidate || a->bit_verified) && (!a->inplace_candidate || (a->inplace_verified && win16_control_inplace_or_lookup(w,a->inplace_owner,a->inplace_parent))) && (!a->nav_candidate || (a->nav_verified && win16_control_nav_identity(w,a->nav_owner,a->nav_parent))) && (!a->lookup_popup_candidate || (v->lookup(a->lookup_popup_source)==a->lookup_popup_source_attachment && win16_control_lookup_popup_identity(w,a->lookup_popup_source,a->lookup_popup_source_parent,a->lookup_popup_owner))) && (!a->group_frame_candidate || (a->group_frame_verified && !a->group_frame_retired && GetParent(w)==a->group_frame_parent)) && (!a->panel_frame_candidate || (a->panel_frame_verified && !a->panel_frame_retired && GetParent(w)==a->panel_frame_parent));
}
void win16_attachment_scroll_changed(HWND w,const win16_attachment_lifecycle_services *v)
{
    if(GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() && v->lookup(w)){
        if(style_control_class_is(w,L"SCROLLBAR"))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
        else v->scrollbars_paint(w,NULL,NULL);
    }
}
BOOL win16_attachment_remove(HWND w,const win16_attachment_lifecycle_services *v)
{
    attachment *a;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    a=v->lookup(w);if(!a){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    ++a->calls;
    RemoveWindowSubclass(w,v->proc,(UINT_PTR)v->proc);
    a->detached=TRUE;v->report_unbind(a);v->cool_unbind(a);
    /* Existing owners handle reentrant destruction/removal independently. */
    v->frame_remove_owned(w,a);
    v->control_remove_owned(w,a);
    if(a->themed)v->theme_remove(a->themed,a);
    v->theme_remove(w,NULL);
    style_control_protocol_release(a->protocol);a->protocol=NULL;
    v->release(a);return TRUE;
}
BOOL win16_attachment_terminal(HWND w,const win16_attachment_lifecycle_services *v)
{
    attachment *a;
    DWORD saved=GetLastError(),error=ERROR_SUCCESS;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    a=v->lookup(w);
    if(a){
        ++a->calls;
        if(RemoveWindowSubclass(w,v->proc,(UINT_PTR)v->proc))a->detached=TRUE;
        else error=ERROR_INVALID_STATE;
    }
    /* These leaves only retire our exact nodes. They do not restore native
     * appearance, send application messages, or call a saved procedure. */
    if(!v->frame_terminal(w) && !error)error=GetLastError();
    if(!v->control_terminal(w) && !error)error=GetLastError();
    if(a && a->themed && a->themed!=w &&
            !v->theme_terminal(a->themed,a) && !error)error=GetLastError();
    if(!v->theme_terminal(w,NULL) && !error)error=GetLastError();
    if(a){v->report_unbind(a);v->cool_unbind(a);style_control_protocol_release(a->protocol);a->protocol=NULL;v->release(a);}
    SetLastError(error?error:saved);return !error;
}

LRESULT win16_attachment_report_notify(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data,const win16_attachment_notify_services *v)
{
    DWORD entry=GetLastError();attachment *a=(attachment *)data;BOOL draw=FALSE;HWND list=a->report_window;HDC dc=NULL;DWORD stage=0;
    ntw16s_vb_paint file_paint;BOOL file_active=FALSE;
    checklist_draw_scope checklist_scope;BOOL checklist_active=FALSE;
    ++a->calls;
    if(a->bit_candidate && !a->detached && m==WM_DRAWITEM && l && GetParent(list)==w && win16_control_bit_identity(list,a->report_module)){
        const DRAWITEMSTRUCT *item=(const DRAWITEMSTRUCT *)l;
        if(item->CtlType==ODT_BUTTON && item->itemAction==ODA_DRAWENTIRE && item->hwndItem==list && item->hDC){
            HWND target=WindowFromDC(item->hDC);
            if(!target || target==list){SetLastError(entry);LRESULT result=v->bit_draw_dispatch(a,w,m,p,l,item);DWORD error=GetLastError();v->release(a);SetLastError(error);return result;}
        }
    }
    if(a->checklist && !a->detached && m==WM_DRAWITEM && l && GetParent(list)==w && win16_control_checklist_identity(list)){
        const DRAWITEMSTRUCT *item=(const DRAWITEMSTRUCT *)l;
        if(item->CtlType==ODT_LISTBOX && item->hwndItem==list && item->hDC && item->itemID!=(UINT)-1 &&
            WindowFromDC(item->hDC)==list){
            checklist_scope=(checklist_draw_scope){(*v->checklist_current),a,item->hDC,item->rcItem};
            (*v->checklist_current)=&checklist_scope;checklist_active=TRUE;
        }
    }
    if((a->file_owner || a->button_owner) && !a->detached && m==WM_DRAWITEM && l && GetParent(list)==w &&
            (a->button_owner?ntw16s_vb_graphical_button(list):ntw16s_vb_owner_file(list))){
        DRAWITEMSTRUCT *item=(DRAWITEMSTRUCT *)l;
        if(item->hwndItem==list && item->hDC && item->CtlType==(a->button_owner?ODT_BUTTON:style_control_vb_drive(list)?ODT_COMBOBOX:ODT_LISTBOX)){
            HWND target=WindowFromDC(item->hDC);BOOL valid=!target || target==list;
            if(!valid && style_control_vb_drive(list)){
                COMBOBOXINFO combo={.cbSize=sizeof(combo)};
                valid=GetComboBoxInfo(list,&combo) && combo.hwndCombo==list && target==combo.hwndList &&
                    GetWindowThreadProcessId(target,NULL)==GetCurrentThreadId();
            }
            if(valid && !a->detached){
                win16_control_vb_paint_begin(&file_paint,list,WM_PRINTCLIENT,&a->fonts,v->user_label_target);file_active=file_paint.active;
                if(file_active){HWND painted=target?target:list;
                    ntw16s_report_paint_enable(&file_paint,painted,a->report_module,&a->detached);
                    ntw16s_report_paint_bind(painted,item->hDC);
                }
            }
        }
    }
    if(!a->file_owner && !a->button_owner && !a->checklist && !a->bit_candidate && m==WM_NOTIFY && l){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==list && header->code==NM_CUSTOMDRAW){
            NMCUSTOMDRAW *cd=(NMCUSTOMDRAW *)l;dc=cd->hdc;stage=cd->dwDrawStage;draw=TRUE;
            ntw16s_report_paint_bind(list,NULL);
        }
    }
    if(m==WM_NCDESTROY){RemoveWindowSubclass(w,v->notify,id);a->report_parent=NULL;}
    SetLastError(entry);LRESULT result=DefSubclassProc(w,m,p,l);DWORD error=GetLastError();
    if(checklist_active)(*v->checklist_current)=checklist_scope.previous;
    if(file_active)ntw16s_vb_paint_end(&file_paint);
    if(draw && !a->detached && !(result&CDRF_SKIPDEFAULT) &&
        (stage==CDDS_PREPAINT || stage==CDDS_ITEMPREPAINT || stage==(CDDS_ITEMPREPAINT|CDDS_SUBITEM)))
        ntw16s_report_paint_bind(list,dc);
    v->release(a);SetLastError(error);return result;
}

BOOL win16_attachment_classify_initial(HWND w,DWORD original_style,BOOL ready,BOOL checklist,BOOL status,BOOL spin,win16_attachment_initial_classification *v)
{
    *v=(win16_attachment_initial_classification){0};
    v->edit=(original_style&WS_CHILD) && style_control_class_is(w,L"EDIT");
    v->control=(original_style&WS_CHILD) && (v->edit || style_control_class_is(w,L"BUTTON") || style_control_class_is(w,L"STATIC") ||
        style_control_class_is(w,L"LISTBOX") || style_control_class_is(w,L"COMBOBOX") || style_control_class_is(w,L"SCROLLBAR") || style_control_class_is(w,L"SysLink"));
    if(checklist)v->control=FALSE;
    if(!v->control && !checklist && !status && !spin && (original_style&WS_CHILD)){
        SetLastError(ERROR_SUCCESS);
        v->protocol=style_control_protocol_bind(w);
        if(!v->protocol && GetLastError()==ERROR_NOT_ENOUGH_MEMORY)return FALSE;
        v->report=v->protocol && style_control_protocol_list_module(w);
        v->tree=v->protocol && style_control_protocol_tree_module(w);
        v->legacy_progress=v->protocol && style_control_protocol_legacy_progress(w);
        v->control=v->protocol && !v->report && !v->legacy_progress;
        v->edit=v->protocol && style_control_protocol_is(w,L"EDIT");
    }
    v->owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    /* These owner-drawn VCL file lists expose LISTBOX as their native class;
     * keep their client callbacks and route only the frame through grid. */
    if(ready && v->control && !v->protocol && (style_control_class_is(w,L"TDirectoryListBox") || style_control_class_is(w,L"TFileListBox")) &&
       win16_control_grid_identity(w,v->owner,GetParent(w))){v->control=FALSE;v->edit=FALSE;}
    return TRUE;
}

static HMODULE native_control_owner(HWND w,WNDPROC proc,const void *address,const WCHAR *class_name)
{
    WNDCLASSEXW native={.cbSize=sizeof(native)};HMODULE caller=NULL;
    if(!GetClassInfoExW(NULL,class_name,&native) || proc!=native.lpfnWndProc ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)address,&caller))return NULL;
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    return owner && caller==owner?owner:NULL;
}
HMODULE win16_attachment_observe_native(HWND w,UINT message,WNDPROC proc,const void *address,
    attachment *a,HWND paint_window,attachment *cool_owner,HWND cool_window,
    WNDPROC *cool_native,attachment *(*lookup)(HWND))
{
    WNDCLASSEXW native={.cbSize=sizeof(native)};HMODULE caller=NULL,native_owner=NULL,owner;
    if(message==WM_PAINT && cool_owner && cool_window==w && !cool_owner->detached && lookup(w)==cool_owner &&
       GetClassInfoExW(NULL,REBARCLASSNAMEW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) &&
       caller==cool_owner->cool_owner)*cool_native=proc;
    if(message==WM_PAINT && a && paint_window==w &&
       a->status_candidate && !a->detached && !a->status_attempted && lookup(w)==a){
        owner=native_control_owner(w,proc,address,STATUSCLASSNAMEW);
        if(owner)a->status_witness_owner=owner;
    }
    if(message==WM_PAINT && a && paint_window==w &&
       a->date_candidate && !a->detached && !a->date_attempted && lookup(w)==a){
        owner=native_control_owner(w,proc,address,DATETIMEPICK_CLASSW);
        if(owner)a->date_witness_owner=owner;
    }
    if(message==WM_PAINT && a && paint_window==w &&
       a->color_candidate && !a->detached && !a->color_attempted && lookup(w)==a){
        owner=native_control_owner(w,proc,address,L"COMBOBOX");
        if(owner)a->color_witness_owner=owner;
    }
    if(message==WM_PAINT && a && paint_window==w &&
       a->track_candidate && !a->detached && !a->track_attempted && lookup(w)==a){
        owner=native_control_owner(w,proc,address,TRACKBAR_CLASSW);
        if(owner)a->track_witness_owner=owner;
    }
    if(message==WM_PAINT && a && paint_window==w &&
       a->scroll_candidate && !a->detached &&
       !a->scroll_attempted && lookup(w)==a &&
       GetParent(w)==a->scroll_parent){
        owner=native_control_owner(w,proc,address,L"SCROLLBAR");
        if(owner)a->scroll_witness=owner;
    }
    if(message==WM_PAINT && a && paint_window==w &&
       a->edit_candidate && !a->detached &&
       !a->edit_attempted && lookup(w)==a){
        owner=native_control_owner(w,proc,address,a->link_candidate?WC_LINK:a->combo_candidate?L"COMBOBOX":L"EDIT");
        if(owner){WCHAR name[64];
            if(!a->combo_candidate || (GetClassNameW(w,name,ARRAYSIZE(name)) && lstrcmpW(name,L"TDBComboBox")))
                a->edit_witness_owner=owner;
        }
    }
    if(message==WM_PAINT && a && paint_window==w &&
       a->group_button_candidate && !a->detached &&
       !a->group_button_attempted && lookup(w)==a &&
       win16_control_group_button_identity(w)){
        owner=native_control_owner(w,proc,address,L"BUTTON");
        if(owner)a->group_button_witness_owner=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PRINTCLIENT && a && paint_window==w &&
       a->vcl_toolbar_candidate && !a->detached &&
       !a->vcl_toolbar_attempted && lookup(w)==a &&
       win16_control_vcl_toolbar_identity(w,a->vcl_toolbar_owner,a->vcl_toolbar_parent) &&
       GetClassInfoExW(NULL,TOOLBARCLASSNAMEW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) &&
       caller==a->vcl_toolbar_owner)a->vcl_toolbar_witness=caller;
    if(message==WM_PAINT && a && paint_window==w &&
       a->tab_candidate && !a->detached &&
       !a->tab_attempted && lookup(w)==a &&
       GetParent(w)==a->tab_parent){
        owner=native_control_owner(w,proc,address,WC_TABCONTROLW);
        if(owner)a->tab_witness_owner=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PAINT && a && paint_window==w &&
       a->vcl_report_candidate && !a->detached &&
       !a->vcl_report_attempted && lookup(w)==a &&
       GetParent(w)==a->vcl_report_parent &&
       GetClassInfoExW(NULL,a->vcl_report_tree?WC_TREEVIEWW:WC_LISTVIEWW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) &&
       caller==a->vcl_report_owner &&
       (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==caller)a->vcl_report_witness=caller;
    native.cbSize=sizeof(native);
    if(message!=WM_PAINT || !a || paint_window!=w ||
       a->detached || lookup(w)!=a ||
       !GetClassInfoExW(NULL,L"STATIC",&native) || proc!=native.lpfnWndProc ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&caller) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)(UINT_PTR)proc,&native_owner))return NULL;
    owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(!owner || caller!=owner || !native_owner)return NULL;
    return native_owner;
}

void win16_attachment_classify_vcl(HWND w,DWORD original_style,BOOL ready,BOOL picture,BOOL checklist,BOOL status,BOOL spin,
    const win16_attachment_initial_classification *i,win16_control_vcl_candidates *v)
{
    *v=(win16_control_vcl_candidates){0};
    if(!i->control && !i->report && !i->tree && !picture && !checklist && !i->legacy_progress && !status && !spin &&
       (original_style&WS_CHILD) && i->owner)win16_control_vcl_classify(w,original_style,ready,i->owner,v);
}
int win16_attachment_eligibility(DWORD original_style,BOOL picture,BOOL checklist,BOOL status,BOOL spin,BOOL vcl_mdi_probe,
    const win16_attachment_initial_classification *i,const win16_control_vcl_candidates *v,const win16_control_vcl_extra_candidates *e)
{
    if(!i->control && !i->report && !i->tree && !picture && !checklist && !i->legacy_progress && !status && !spin && !v->static_candidate && !v->scroll_candidate && !v->edit_candidate && !v->group_button_candidate && !v->grid_candidate && !v->inplace_candidate && !v->nav_candidate && !v->vcl_toolbar_candidate && !v->track_candidate && !v->tab_candidate && !v->vcl_report_candidate && !v->color_candidate && !v->date_candidate && !v->status_candidate && !v->bit_candidate && !v->group_frame_candidate && !v->panel_frame_candidate && (original_style&WS_CHILD) &&
            i->owner && i->owner!=GetModuleHandleW(NULL) && !e->vb_mdi_child && !vcl_mdi_probe && !e->vb_user_edge && !e->vb_data_edge && !e->rich_candidate && !e->box_candidate && !e->controlbar_candidate && !e->cool_candidate){
        return 1;
    }
    /* A custom client-only child and MDICLIENT keep application painting.
     * MDI child frames have WS_CAPTION and take the ordinary frame route. */
    if(!i->control && !i->report && !picture && !checklist && !i->legacy_progress && !status && !spin && !v->static_candidate && !v->scroll_candidate && !v->edit_candidate && !v->group_button_candidate && !v->grid_candidate && !v->inplace_candidate && !v->nav_candidate && !v->vcl_toolbar_candidate && !v->track_candidate && !v->tab_candidate && !v->vcl_report_candidate && !v->color_candidate && !v->date_candidate && !v->status_candidate && !v->bit_candidate && !v->group_frame_candidate && !v->panel_frame_candidate && (original_style&WS_CHILD) && !(original_style&(WS_CAPTION|WS_THICKFRAME)) && !e->vb_user_edge && !e->vb_data_edge && !e->rich_candidate && !e->box_candidate && !e->controlbar_candidate && !e->cool_candidate){
        return 2;
    }
    return 0;
}

void win16_attachment_initialize(HWND w,DWORD original_style,attachment *a,const win16_attachment_seed *seed)
{
    const win16_attachment_initial_classification *i=seed->initial;
    const win16_control_vcl_candidates *v=seed->vcl;
    const win16_control_vcl_extra_candidates *e=seed->extra;
    a->calls=1;a->cool_candidate=e->cool_candidate;a->cool_parent=GetParent(w);a->cool_owner=i->owner;if(e->cool_candidate)a->report_window=w;a->mdi_candidate=seed->vcl_mdi_probe;a->mdi_parent=GetParent(w);a->mdi_host=GetParent(a->mdi_parent);a->mdi_owner=i->owner;a->mdi_host_attachment=seed->vcl_mdi_probe?seed->lookup(a->mdi_host):NULL;if(a->mdi_host_attachment)++a->mdi_host_attachment->calls;a->nav_candidate=v->nav_candidate;a->nav_parent=GetParent(w);a->nav_owner=i->owner;a->lookup_popup_candidate=seed->lookup_popup_candidate;a->lookup_popup_source=seed->popup_source;a->lookup_popup_source_parent=seed->popup_source_parent;a->lookup_popup_owner=i->owner;a->lookup_popup_source_attachment=seed->lookup_popup_candidate?seed->popup_source_attachment:NULL;if(a->lookup_popup_source_attachment)++a->lookup_popup_source_attachment->calls;a->controlbar_candidate=e->controlbar_candidate;a->controlbar_parent=GetParent(w);a->controlbar_owner=i->owner;a->rich_popup=e->rich_popup;a->rich_month=e->rich_month;a->rich_hotkey=e->rich_hotkey;a->box_candidate=e->box_candidate;a->box_owner=i->owner;a->box_parent=GetParent(w);a->rich_unicode=e->rich_unicode;a->rich_candidate=e->rich_candidate;a->rich_parent=e->rich_popup?GetDesktopWindow():GetParent(w);a->rich_owner=i->owner;a->group_frame_candidate=v->group_frame_candidate;a->panel_frame_candidate=v->panel_frame_candidate;a->vb_user_candidate=e->vb_user_edge;a->vb_data_candidate=e->vb_data_edge;a->protocol=i->protocol;a->file_owner=ntw16s_vb_owner_file(w);a->picture=seed->picture;a->button_owner=ntw16s_vb_graphical_button(w);a->checklist=seed->checklist;a->status=seed->status;a->spin=seed->spin;a->modern_spin=seed->spin&&style_control_vcl_modern_spin_shape(w);a->static_candidate=v->static_candidate;
    a->scroll_candidate=v->scroll_candidate;a->scroll_parent=GetParent(w);a->edit_candidate=v->edit_candidate;a->link_candidate=v->edit_candidate && style_control_class_is(w,L"TLinkLabel");a->combo_candidate=v->edit_candidate && (style_control_class_is(w,L"TComboBox") || style_control_class_is(w,L"TDBComboBox"));a->combo_parent=GetParent(w);a->combo_original_type=original_style&3;a->spinedit_candidate=v->edit_candidate && style_control_class_is(w,L"TSpinEdit");a->group_button_candidate=v->group_button_candidate;a->grid_candidate=v->grid_candidate;a->inplace_candidate=v->inplace_candidate;a->inplace_owner=i->owner;a->inplace_parent=GetParent(w);a->vcl_toolbar_candidate=v->vcl_toolbar_candidate;a->vcl_toolbar_owner=i->owner;a->vcl_toolbar_parent=GetParent(w);a->grid_owner=i->owner;a->grid_parent=GetParent(w);a->edit_original_style=original_style;a->track_candidate=v->track_candidate;a->tab_candidate=v->tab_candidate;a->tab_parent=GetParent(w);a->vcl_report_candidate=v->vcl_report_candidate;a->vcl_report_parent=GetParent(w);a->vcl_report_owner=i->owner;a->vcl_report_tree=v->vcl_report_candidate && style_control_class_is(w,L"TTreeView");a->color_candidate=v->color_candidate;a->date_candidate=v->date_candidate;a->status_candidate=v->status_candidate;a->bit_candidate=v->bit_candidate;
    if(i->report || i->tree || a->file_owner || a->button_owner || a->checklist || a->bit_candidate){a->report_window=w;a->tree=i->tree;
        a->report_module=(a->file_owner || a->button_owner || a->bit_candidate)?(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE):i->tree?style_control_protocol_tree_module(w):style_control_protocol_list_module(w);a->report_parent=GetParent(w);}
}

BOOL win16_attachment_static_paint(const attachment *a,UINT m)
{
    return ((a->static_candidate || a->scroll_candidate || a->edit_candidate || a->group_button_candidate || a->grid_candidate || a->inplace_candidate || a->nav_candidate || a->vcl_toolbar_candidate || a->track_candidate || a->tab_candidate || a->vcl_report_candidate || a->color_candidate || a->date_candidate || a->status_candidate || a->rich_candidate || a->group_frame_candidate || a->panel_frame_candidate) && !a->detached && m==WM_PAINT);
}
BOOL win16_attachment_spin_message(const attachment *a,UINT m)
{
    return a->spin && !a->detached && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT ||
        m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || m==WM_MOUSEMOVE || m==WM_ENABLE || m==WM_CANCELMODE ||
        m==WM_SETFOCUS || m==WM_KILLFOCUS);
}
void win16_attachment_prepare_report(HWND w,UINT m,WPARAM p,attachment *a,ntw16s_vb_paint *paint,void (*bind)(HWND,attachment *))
{
    (void)m;
    if(a->file_owner || a->picture || a->button_owner || a->vb_user_candidate)paint->cancelled=&a->detached;
    if(paint->native_active)paint->native_dc=(HDC)p;
    if(paint->active && !a->detached && !a->protocol && !a->vcl_report_candidate)bind(w,a);
    if(paint->active && a->report_module && !a->file_owner && !a->detached){
        DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        if((a->tree || (((style&LVS_TYPEMASK)==LVS_REPORT || a->vcl_report_candidate) &&
            !(style&(LVS_OWNERDRAWFIXED|LVS_OWNERDATA)) && !SendMessageW(w,LVM_ISGROUPVIEWENABLED,0,0) &&
            !a->detached && (DWORD)GetWindowLongPtrW(w,GWL_STYLE)==style)) && !a->detached)
            ntw16s_report_paint_enable(paint,w,a->report_module,&a->detached);
    }
}
void win16_attachment_reset_witness(attachment *a,UINT m)
{
    if(m==WM_NCPAINT && a->controlbar_candidate)a->controlbar_verified=FALSE;
    if(m==WM_PAINT && a->rich_candidate && (!a->rich_installed || a->rich_month))a->rich_witness=FALSE;
}
win16_attachment_dispatch win16_attachment_dispatch_kind(HWND w,attachment *a,UINT m,WPARAM p,LPARAM l,
    BOOL status_scope,BOOL print_clip,BOOL spin_message,BOOL nested_static)
{
    if(a->cool_candidate && !a->detached && m==WM_PAINT)return WIN16_ATTACHMENT_DISPATCH_COOL;
    if(m==WM_PRINTCLIENT && status_scope)return WIN16_ATTACHMENT_DISPATCH_STATUS;
    if(print_clip)return WIN16_ATTACHMENT_DISPATCH_COLOR;
    if(a->modern_spin && spin_message)return WIN16_ATTACHMENT_DISPATCH_SPIN;
    if(((a->box_candidate && !a->box_installed) || a->controlbar_candidate) && !a->detached && m==WM_NCPAINT)return WIN16_ATTACHMENT_DISPATCH_BOX;
    if(a->inplace_candidate && !a->detached && (m==WM_LBUTTONDOWN || (m==WM_SYSKEYDOWN && p==VK_DOWN && ((ULONG_PTR)l&((ULONG_PTR)1<<29)))) && win16_control_lookup_identity(w,a->inplace_owner,a->inplace_parent))return WIN16_ATTACHMENT_DISPATCH_LOOKUP;
    if(nested_static || win16_attachment_static_paint(a,m))return WIN16_ATTACHMENT_DISPATCH_STATIC;
    return WIN16_ATTACHMENT_DISPATCH_NATIVE;
}

BOOL win16_attachment_mdi_identity(HWND w,HWND parent,HWND host,HMODULE owner,attachment *(*lookup)(HWND))
{
    DWORD pid=0,host_pid=0,client_pid=0;
    return owner && parent && host && IsWindow(w) && IsWindow(parent) && IsWindow(host) &&
        GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,&client_pid)==GetCurrentThreadId() && client_pid==pid &&
        GetWindowThreadProcessId(host,&host_pid)==GetCurrentThreadId() && host_pid==pid &&
        GetParent(w)==parent && GetParent(parent)==host && style_control_class_is(w,L"TForm") &&
        style_control_class_is(parent,L"MDIClient") && style_control_class_is(host,L"TForm") &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==GetModuleHandleW(L"user32.dll") &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        (HMODULE)GetClassLongPtrW(host,GCLP_HMODULE)==owner && lookup(host) &&
        (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_MDICHILD) &&
        (GetWindowLongPtrW(w,GWL_STYLE)&WS_CAPTION)==WS_CAPTION;
}

BOOL win16_attachment_combo_same_role(HWND w,attachment *a)
{return win16_control_combo_identity(w,a->combo_parent) && ((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==a->combo_original_type;}

BOOL win16_attachment_vcl_combo_candidate(HWND w,attachment *a)
{(void)w;return a && !a->detached && a->combo_candidate;}

BOOL win16_attachment_vcl_combo_valid(HWND w,attachment *a)
{
    return !a || a->detached || !a->combo_candidate || !a->edit_installed || win16_attachment_combo_same_role(w,a);
}

BOOL win16_attachment_group_button_valid(HWND w,attachment *a)
{

    return !a || a->detached || !a->group_button_candidate || !a->group_button_installed ||
        (a->group_button_parent==GetParent(w) && win16_control_group_button_identity(w));
}

BOOL win16_attachment_group_push_button(HWND w,attachment *a)
{
    WCHAR name[64];
    return a && !a->detached && a->group_button_candidate && a->group_button_installed &&
        a->group_button_parent==GetParent(w) && win16_control_group_button_identity(w) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TButton");
}

BOOL win16_attachment_group_choice_button(HWND w,attachment *a)
{
    WCHAR name[64];
    return a && !a->detached && a->group_button_candidate && a->group_button_installed &&
        a->group_button_parent==GetParent(w) && win16_control_group_button_identity(w) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TCheckBox") || !lstrcmpW(name,L"TRadioButton") || !lstrcmpW(name,L"TGroupButton"));
}

BOOL win16_attachment_vb_data_valid(HWND w,attachment *a)
{

    return !a || a->detached || !a->vb_data_edge || win16_control_vb_data_role(w,a->vb_data_parent);
}

BOOL win16_attachment_vb_data_attached(HWND w,attachment *a)
{

    return a && !a->detached && a->vb_data_edge && win16_control_vb_data_role(w,a->vb_data_parent);
}

BOOL win16_attachment_vb_user_label_target(HWND w,attachment *a)
{

    if(!a || a->detached || !a->vb_user_candidate)return FALSE;
    HWND parent=GetParent(w);
    return win16_control_vb_user_edge_role(w,parent) && (!a->vb_user_parent || a->vb_user_parent==parent);
}

BOOL win16_attachment_vb_user_edge_dormant(HWND w,attachment *a)
{

    return a && !a->detached && a->vb_user_edge && win16_control_vb_user_edge_role(w,a->vb_user_parent) &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE);
}

BOOL win16_attachment_vb_user_edge_valid(HWND w,attachment *a)
{

    return !a || a->detached || !a->vb_user_edge || win16_control_vb_user_edge_identity(w,a->vb_user_parent);
}

BOOL win16_attachment_grid_valid(HWND w,attachment *a)
{

    return !a || a->detached || !a->grid_candidate || !a->grid_installed || win16_control_grid_identity(w,a->grid_owner,a->grid_parent);
}

BOOL win16_attachment_vcl_toolbar_valid(HWND w,attachment *a)
{

    return !a || a->detached || !a->vcl_toolbar_candidate || !a->vcl_toolbar_installed ||
        win16_control_vcl_toolbar_identity(w,a->vcl_toolbar_owner,a->vcl_toolbar_parent);
}

BOOL win16_attachment_vcl_tab_valid(HWND w,attachment *a)
{
    WCHAR name[64];
    return !a || a->detached || !a->tab_candidate || !a->tab_installed ||
        (GetParent(w)==a->tab_parent && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TTabControl") || !lstrcmpW(name,L"TTabbedNotebook") || !lstrcmpW(name,L"TPageControl")) &&
         !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)));
}

static void db_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address,attachment *a,HWND observed,attachment *(*lookup)(HWND),BOOL candidate,const WCHAR *class_name,const char *native_class)
{
    WNDCLASSEXA native={.cbSize=sizeof(native)};HMODULE caller=NULL,owner;WCHAR name[64];
    if(message!=WM_PAINT || !a || observed!=w || !candidate ||
        a->detached || a->edit_attempted || lookup(w)!=a ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpW(name,class_name) ||
        !GetClassInfoExA(NULL,native_class,&native) || proc!=native.lpfnWndProc ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller))return;
    owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(owner && caller==owner)a->edit_witness_owner=owner;
}

void win16_attachment_dbedit_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address,attachment *a,HWND observed,attachment *(*lookup)(HWND))
{db_ansi_witness(proc,w,message,address,a,observed,lookup,a && a->edit_candidate,L"TDBEdit","EDIT");}

void win16_attachment_dbcombo_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address,attachment *a,HWND observed,attachment *(*lookup)(HWND))
{db_ansi_witness(proc,w,message,address,a,observed,lookup,a && a->combo_candidate,L"TDBComboBox","COMBOBOX");}

BOOL win16_attachment_inplace_paint(attachment *owner,HWND observed,HDC dc)
{return owner && owner->inplace_candidate && !owner->detached && dc && WindowFromDC(dc)==observed;}

BOOL win16_attachment_checklist_draw_target(checklist_draw_scope *s,HDC dc,const RECT *r,const void *address)
{
    return s && s->owner && !s->owner->detached &&
        win16_control_checklist_target(dc,s->dc,s->owner->report_window,s->owner->report_parent,&s->item,r,address,win16_control_checklist_identity);
}

void win16_attachment_box_witness(HWND w,UINT msg,const void *address,attachment *owner,HWND observed,BOOL *witnessed)
{
    HMODULE caller=NULL;
    if(!owner || observed!=w || msg!=WM_NCPAINT || owner->detached ||
       !win16_control_box_identity(w,owner->box_owner,owner->box_parent) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) ||
       (caller!=owner->box_owner && caller!=GetModuleHandleW(L"user32.dll")))return;
    *witnessed=TRUE;
}

BOOL win16_attachment_rich_valid(HWND w,attachment *a,BOOL (*identity)(HWND,attachment *))
{return !a || a->detached || ((!a->rich_candidate || !a->rich_installed || identity(w,a)) && (!a->box_candidate || !a->box_installed || win16_control_box_identity(w,a->box_owner,a->box_parent)));}

BOOL win16_attachment_rich_witness(WNDPROC proc,HWND w,UINT msg,const void *address,BOOL wide,attachment *owner,HWND observed,BOOL (*identity)(HWND,attachment *))
{
    WNDCLASSEXA c={.cbSize=sizeof(c)};WNDCLASSEXW cw={.cbSize=sizeof(cw)};HMODULE caller=NULL,rich;
    if(msg!=WM_PAINT || !owner || observed!=w || !owner->rich_candidate)return FALSE;
    rich=GetModuleHandleW(owner->rich_popup?L"user32.dll":(owner->rich_hotkey || owner->rich_month)?L"comctl32.dll":L"riched20.dll");
    if(!identity(w,owner) || wide!=owner->rich_unicode || !rich ||
       (wide?(!GetClassInfoExW(owner->rich_popup?NULL:rich,owner->rich_popup?L"LISTBOX":owner->rich_month?L"SysMonthCal32":owner->rich_hotkey?L"msctls_hotkey32":L"RichEdit20W",&cw) || proc!=cw.lpfnWndProc):(!GetClassInfoExA(rich,"RichEdit20A",&c) || proc!=c.lpfnWndProc)) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) ||
       caller!=owner->rich_owner)return FALSE;
    owner->rich_witness=TRUE;
    return TRUE;
}

BOOL win16_attachment_cool_identity(HWND w,attachment *a,attachment *(*lookup)(HWND))
{
    DWORD pid=0,ppid=0;HWND parent=a->cool_parent;WNDCLASSEXW c={.cbSize=sizeof(c)};HMODULE proc=NULL;
    return !a->detached && parent && IsWindow(w) && IsWindow(parent) && lookup(w)==a &&
        GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,&ppid)==GetCurrentThreadId() && ppid==pid &&
        style_control_class_is(w,L"TCoolBar") && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==a->cool_owner &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==a->cool_owner &&
        GetClassInfoExW(a->cool_owner,L"TCoolBar",&c) && c.lpfnWndProc &&
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)c.lpfnWndProc,&proc) && proc==a->cool_owner;
}
