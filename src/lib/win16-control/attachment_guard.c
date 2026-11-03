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
