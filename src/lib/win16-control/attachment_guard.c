#include "attachment_state.h"
#include "control_class.h"
#include "vcl_identity.h"

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
    if(a->picture && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !ntw16s_vb_picture(w))
        (void)v->remove(w);
    if(a->button_owner && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !ntw16s_vb_graphical_button(w))
        (void)v->remove(w);
    if(a->file_owner && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY &&
        ntw16s_vb_checklist_class(w) && !ntw16s_vb_checklist_owner(w))
        (void)v->remove(w);
    if((a->file_owner || a->button_owner || a->checklist) && !a->detached && a->report_parent && m!=WM_DESTROY && m!=WM_NCDESTROY && GetParent(w)!=a->report_parent)
        (void)v->remove(w);
    if(a->inplace_candidate && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !win16_control_inplace_or_lookup(w,a->inplace_owner,a->inplace_parent))(void)v->remove(w);
    if(a->lookup_popup_candidate && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && (v->lookup(a->lookup_popup_source)!=a->lookup_popup_source_attachment || !win16_control_lookup_popup_identity(w,a->lookup_popup_source,a->lookup_popup_source_parent,a->lookup_popup_owner)))(void)v->remove(w);
    if(a->mdi_candidate && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && (v->lookup(a->mdi_host)!=a->mdi_host_attachment || !v->mdi_identity(w,a->mdi_parent,a->mdi_host,a->mdi_owner)))(void)v->remove(w);
    if(a->nav_candidate && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !win16_control_nav_identity(w,a->nav_owner,a->nav_parent))(void)v->remove(w);
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
    /* Runtime controls may initially have a temporary parent. Bind the
     * verified border to the current parent only at the first real paint. */
    if(m==WM_PAINT && a->vb_data_candidate && !a->vb_data_attempted && !a->detached){
        DWORD saved=GetLastError();HWND parent=GetParent(w);
        if(win16_control_vb_data_role(w,parent)){
            a->vb_data_attempted=TRUE;a->vb_data_parent=parent;a->vb_data_edge=TRUE;
            BOOL installed=v->grid_install(w,NULL,a,&a->detached);
            if(installed && !a->detached && win16_control_vb_data_role(w,parent))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
            else if(!a->detached){a->vb_data_edge=FALSE;v->remove_owned(w,a);}
        }
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->vb_user_candidate && !a->vb_user_attempted && !a->detached){
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
    if(m==WM_PAINT && a->rich_candidate && !a->rich_installed && a->rich_witness && v->rich_identity(w,a)){
        DWORD saved=GetLastError();a->rich_installed=TRUE;
        if((a->rich_month || v->grid_install(w,NULL,a,&a->detached)) && !a->detached && v->rich_identity(w,a)){if(!a->rich_month)RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);}
        else if(!a->detached){a->rich_installed=FALSE;v->remove_owned(w,a);}
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->grid_candidate && !a->grid_attempted && !a->detached){
        DWORD saved=GetLastError();SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_ALL};DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        a->grid_attempted=TRUE;
        if(win16_control_grid_identity(w,a->grid_owner,a->grid_parent) &&
            ((ntw16s_class_is(w,L"TValueListEditor") && !(style&(WS_HSCROLL|WS_VSCROLL))) ||
             (v->get_scroll_info(w,(style&WS_HSCROLL)?SB_HORZ:SB_VERT,&info) &&
              info.nMin<=info.nMax && info.nPos>=info.nMin && info.nPos<=info.nMax)) && !a->detached){
            BOOL installed=v->grid_install(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->grid_installed=TRUE;RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);}
            else if(!a->detached)v->remove_owned(w,a);
        }
        SetLastError(saved);
    }
}

void win16_attachment_post_toolbar(HWND w,UINT m,attachment *a,const win16_attachment_verified_services *v)
{
    if(m==WM_PAINT && a->vcl_toolbar_candidate && !a->vcl_toolbar_attempted && !a->detached && a->vcl_toolbar_witness){
        DWORD saved=GetLastError();a->vcl_toolbar_attempted=TRUE;
        ntw16s_control_protocol *protocol=win16_control_vcl_toolbar_identity(w,a->vcl_toolbar_owner,a->vcl_toolbar_parent)?ntw16s_control_protocol_bind_verified_toolbar(w,a->vcl_toolbar_witness):NULL;
        if(protocol && !a->detached){
            a->protocol=protocol;BOOL installed=v->toolbar_install(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->vcl_toolbar_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){v->remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
}
void win16_attachment_post_verified(HWND w,UINT m,attachment *a,const win16_attachment_verified_services *v)
{
    if(m==WM_PAINT && a->group_button_candidate && !a->group_button_attempted && !a->detached && a->group_button_witness_owner){
        DWORD saved=GetLastError();a->group_button_attempted=TRUE;
        ntw16s_control_protocol *protocol=win16_control_group_button_identity(w)?ntw16s_control_protocol_bind_verified_group_button(w,a->group_button_witness_owner):NULL;
        if(protocol && !a->detached){
            a->protocol=protocol;
            BOOL installed=v->control_install(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached){a->group_button_parent=GetParent(w);a->group_button_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){v->remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->scroll_candidate && !a->scroll_attempted && !a->detached && a->scroll_witness){
        DWORD saved=GetLastError();a->scroll_attempted=TRUE;
        ntw16s_control_protocol *protocol=GetParent(w)==a->scroll_parent?ntw16s_control_protocol_bind_verified_scroll(w,a->scroll_witness):NULL;
        if(protocol && !a->detached && GetParent(w)==a->scroll_parent){a->protocol=protocol;
            BOOL installed=v->control_install(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached && GetParent(w)==a->scroll_parent){a->scroll_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){v->remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->edit_candidate && !a->edit_attempted && !a->detached && a->edit_witness_owner){
        DWORD saved=GetLastError();a->edit_attempted=TRUE;
        ntw16s_control_protocol *protocol=a->combo_candidate && !v->combo_same_role(w,a)?NULL:a->link_candidate?ntw16s_control_protocol_bind_verified_link(w,a->edit_witness_owner):a->combo_candidate?ntw16s_control_protocol_bind_verified_combo(w,a->edit_witness_owner):ntw16s_control_protocol_bind_verified_edit(w,a->edit_witness_owner);
        if(protocol && !a->detached && (!a->combo_candidate || v->combo_same_role(w,a))){
            a->protocol=protocol;
            BOOL installed=v->control_install(w,!a->link_candidate && !a->combo_candidate,(a->edit_original_style&WS_BORDER)!=0,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached && !a->link_candidate && !a->combo_candidate && !a->spinedit_candidate && (a->edit_original_style&ES_MULTILINE)){
                a->themed=w;
                installed=v->theme_enable(w,a) && v->edit_scrollbars_enable(w);
            }
            if(installed && !a->detached && (!a->combo_candidate || v->combo_same_role(w,a))){a->edit_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){v->remove_owned(w,a);if(a->themed){v->theme_remove(w,a);a->themed=NULL;}
                ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        /* An empty link has no ideal height. Retry on a later paint only
         * after a fresh same-owner native SysLink forwarding witness. */
        if(a->link_candidate && !a->detached && !a->edit_installed){
            a->edit_attempted=FALSE;a->edit_witness_owner=NULL;
        }
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->vcl_report_candidate && !a->vcl_report_attempted && !a->detached && a->vcl_report_witness)v->vcl_report_try_bind(w,a);
    if(m==WM_PAINT && a->tab_candidate && !a->tab_attempted && !a->detached && a->tab_witness_owner){
        DWORD saved=GetLastError();a->tab_attempted=TRUE;
        ntw16s_control_protocol *protocol=GetParent(w)==a->tab_parent?ntw16s_control_protocol_bind_verified_tab(w,a->tab_witness_owner):NULL;
        if(protocol && !a->detached){a->protocol=protocol;
            BOOL installed=v->control_install(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached){a->tab_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){v->remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->track_candidate && !a->track_attempted && !a->detached && a->track_witness_owner){
        DWORD saved=GetLastError();a->track_attempted=TRUE;
        ntw16s_control_protocol *protocol=ntw16s_control_protocol_bind_verified_trackbar(w,a->track_witness_owner);
        if(protocol && !a->detached){
            a->protocol=protocol;
            BOOL installed=v->trackbar_install(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->track_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){v->remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
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

BOOL win16_attachment_install_style(HWND w,attachment *a,ntw16s_control_protocol *protocol,DWORD original_style,BOOL legacy_progress,BOOL tree,BOOL picture,BOOL control,BOOL edit,BOOL report,DWORD *out_error,const win16_attachment_install_services *v)
{
    BOOL installed;DWORD error;
    if(a->cool_candidate || a->box_candidate || a->controlbar_candidate || a->rich_candidate || a->vb_data_candidate || a->vb_user_candidate || a->checklist || a->status || a->spin || a->static_candidate || a->scroll_candidate || a->edit_candidate || a->group_button_candidate || a->grid_candidate || a->inplace_candidate || a->nav_candidate || a->vcl_toolbar_candidate || a->track_candidate || a->tab_candidate || a->vcl_report_candidate || a->color_candidate || a->date_candidate || a->status_candidate || a->bit_candidate || a->group_frame_candidate || a->panel_frame_candidate)installed=TRUE;
    else if(legacy_progress)installed=v->legacy_progress(w,NULL,a,&a->detached);
    else if(protocol && ntw16s_control_protocol_toolbar(w))
        installed=v->toolbar(w,NULL,a,&a->detached);
    else if(protocol && ntw16s_control_protocol_is(w,UPDOWN_CLASSW))installed=v->updown(w,NULL,a,&a->detached);
    else if(protocol && ntw16s_control_protocol_is(w,TRACKBAR_CLASSW))installed=v->trackbar(w,NULL,a,&a->detached);
    else if(protocol && ntw16s_control_protocol_is(w,PROGRESS_CLASSW))installed=v->progress(w,NULL,a,&a->detached);
    else if(tree)installed=v->tree(w,NULL,a,&a->detached);
    else if(picture)installed=v->picture(w,NULL,a,&a->detached);
    else if(a->lookup_popup_candidate)installed=v->grid(w,NULL,a,&a->detached);
    else if(a->button_owner)installed=v->frame_scrollbars(w,NULL,a,&a->detached);
    else if(control)installed=v->control(w,edit,(original_style&WS_BORDER)!=0,
        ntw16s_class_is(w,L"LISTBOX") && (original_style&LBS_USETABSTOPS) && !a->file_owner,NULL,a,&a->detached);
    else if(report && !(original_style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION)))installed=TRUE;
    else installed=v->frame(w,NULL,a,&a->detached);
    error=GetLastError();
    if(installed && !a->detached){
        if(control){
            HWND themed=NULL;BOOL list=FALSE;
            if(!a->file_owner && ntw16s_class_is(w,L"COMBOBOX")){
                COMBOBOXINFO info={.cbSize=sizeof(info)};
                if(v->get_combo_info(w,&info)){themed=info.hwndList;list=TRUE;}
            }else if(ntw16s_class_is(w,L"LISTBOX")){themed=w;list=TRUE;}
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
    ntw16s_control_protocol *protocol=ntw16s_control_protocol_bind(w);
    if(!protocol)return;
    HMODULE module=ntw16s_control_protocol_list_module(w);
    BOOL tree=ntw16s_control_protocol_tree_module(w)!=NULL;if(tree)module=ntw16s_control_protocol_tree_module(w);
    HWND parent=GetParent(w);
    if(!a->detached && module && parent &&
        SetWindowSubclass(parent,v->notify,(UINT_PTR)w,(DWORD_PTR)a)){
        a->protocol=protocol;a->report_window=w;a->report_parent=parent;a->report_module=module;a->tree=tree;
    }else ntw16s_control_protocol_release(protocol);
}
/* The runtime-owner path is accepted only after this exact HWND forwards its
 * own paint to the registered native class. Keep the public protocol checks. */
void win16_attachment_vcl_report_bind(HWND w,attachment *a,const win16_attachment_report_services *v)
{
    DWORD saved=GetLastError();a->vcl_report_attempted=TRUE;
    BOOL tree=a->vcl_report_tree;
    ntw16s_control_protocol *protocol=GetParent(w)==a->vcl_report_parent?
        ntw16s_control_protocol_bind_verified_report(w,a->vcl_report_witness,tree):NULL;
    if(protocol && !a->detached){
        HMODULE module=tree?ntw16s_control_protocol_tree_module(w):ntw16s_control_protocol_list_module(w);
        HWND parent=GetParent(w);
        if(module && parent==a->vcl_report_parent && SetWindowSubclass(parent,v->notify,(UINT_PTR)w,(DWORD_PTR)a)){
            a->protocol=protocol;a->report_window=w;a->report_parent=parent;a->report_module=module;a->tree=tree;
            DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
            BOOL installed=tree?v->tree_install(w,NULL,a,&a->detached):
                !(style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION))?TRUE:v->frame_install(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->vcl_report_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){(void)v->remove(w);}
        }else ntw16s_control_protocol_release(protocol);
    }else if(protocol)ntw16s_control_protocol_release(protocol);
    if(!a->detached && !a->vcl_report_installed){a->vcl_report_attempted=FALSE;a->vcl_report_witness=NULL;}
    SetLastError(saved);
}
void win16_attachment_report_unbind(attachment *a,const win16_attachment_report_services *v)
{
    DWORD_PTR value;
    if(a->report_parent && GetWindowSubclass(a->report_parent,v->notify,(UINT_PTR)a->report_window,&value) &&
        value==(DWORD_PTR)a)RemoveWindowSubclass(a->report_parent,v->notify,(UINT_PTR)a->report_window);
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
        if(ntw16s_class_is(w,L"SCROLLBAR"))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
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
    ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;
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
    if(a){v->report_unbind(a);v->cool_unbind(a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;v->release(a);}
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
        if(item->hwndItem==list && item->hDC && item->CtlType==(a->button_owner?ODT_BUTTON:ntw16s_vb_drive(list)?ODT_COMBOBOX:ODT_LISTBOX)){
            HWND target=WindowFromDC(item->hDC);BOOL valid=!target || target==list;
            if(!valid && ntw16s_vb_drive(list)){
                COMBOBOXINFO combo={.cbSize=sizeof(combo)};
                valid=GetComboBoxInfo(list,&combo) && combo.hwndCombo==list && target==combo.hwndList &&
                    GetWindowThreadProcessId(target,NULL)==GetCurrentThreadId();
            }
            if(valid && !a->detached){
                v->paint_begin(&file_paint,list,WM_PRINTCLIENT,&a->fonts);file_active=file_paint.active;
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
    v->edit=(original_style&WS_CHILD) && ntw16s_class_is(w,L"EDIT");
    v->control=(original_style&WS_CHILD) && (v->edit || ntw16s_class_is(w,L"BUTTON") || ntw16s_class_is(w,L"STATIC") ||
        ntw16s_class_is(w,L"LISTBOX") || ntw16s_class_is(w,L"COMBOBOX") || ntw16s_class_is(w,L"SCROLLBAR") || ntw16s_class_is(w,L"SysLink"));
    if(checklist)v->control=FALSE;
    if(!v->control && !checklist && !status && !spin && (original_style&WS_CHILD)){
        SetLastError(ERROR_SUCCESS);
        v->protocol=ntw16s_control_protocol_bind(w);
        if(!v->protocol && GetLastError()==ERROR_NOT_ENOUGH_MEMORY)return FALSE;
        v->report=v->protocol && ntw16s_control_protocol_list_module(w);
        v->tree=v->protocol && ntw16s_control_protocol_tree_module(w);
        v->legacy_progress=v->protocol && ntw16s_control_protocol_legacy_progress(w);
        v->control=v->protocol && !v->report && !v->legacy_progress;
        v->edit=v->protocol && ntw16s_control_protocol_is(w,L"EDIT");
    }
    v->owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    /* These owner-drawn VCL file lists expose LISTBOX as their native class;
     * keep their client callbacks and route only the frame through grid. */
    if(ready && v->control && !v->protocol && (ntw16s_class_is(w,L"TDirectoryListBox") || ntw16s_class_is(w,L"TFileListBox")) &&
       win16_control_grid_identity(w,v->owner,GetParent(w))){v->control=FALSE;v->edit=FALSE;}
    return TRUE;
}
