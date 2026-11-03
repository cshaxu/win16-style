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
