#include "lib-win32-style/application/control/control_vcl_identity.h"
#include "attachment_state.h"
#include "control_class.h"

void win16_attachment_guard_cool(HWND w,UINT m,attachment *a,const win16_attachment_guard_services *v)
{
    if(a->base.runtime.cool.candidate && !a->base.instance.detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !v->cool_identity(w,a))(void)v->remove(w);
}
void win16_attachment_guard_roles(HWND w,UINT m,attachment *a,const win16_attachment_guard_services *v)
{
    if(m!=WM_DESTROY && m!=WM_NCDESTROY){
        if(a->picture && !a->base.instance.detached && !ntw16s_vb_picture(w))
            (void)v->remove(w);
        if(a->button_owner && !a->base.instance.detached && !ntw16s_vb_graphical_button(w))
            (void)v->remove(w);
        if(a->file_owner && !a->base.instance.detached &&
            style_control_vb_checklist_class(w) && !ntw16s_vb_checklist_owner(w))
            (void)v->remove(w);
    }
    style_runtime_roles_guard(w,m,&a->base.instance,&a->base.runtime,a->file_owner || a->button_owner || a->checklist,v->host_attached,v->remove);
}

BOOL win16_attachment_install_render(HWND w,attachment *a,style_control_protocol *protocol,DWORD original_style,BOOL legacy_progress,BOOL tree,BOOL picture,BOOL control,BOOL edit,BOOL report,const win16_attachment_install_services *v)
{
    BOOL installed;
    if(a->base.runtime.cool.candidate || a->base.runtime.box.candidate || a->base.runtime.controlbar.candidate || a->base.runtime.rich.candidate || a->base.runtime.data.candidate || a->base.runtime.user.candidate || a->checklist || a->base.runtime.status.present || a->base.runtime.spin.candidate || a->base.runtime.roles.text.candidate || a->base.runtime.scroll.candidate || a->base.runtime.edit.candidate || a->base.runtime.group.candidate || a->base.runtime.grid.candidate || a->base.runtime.roles.inplace.candidate || a->base.runtime.roles.nav.candidate || a->base.runtime.toolbar.candidate || a->base.runtime.track.candidate || a->base.runtime.tab.candidate || a->base.runtime.report.binding.candidate || a->base.runtime.color.candidate || a->base.runtime.date.candidate || a->base.runtime.status.candidate || a->base.runtime.roles.bit.candidate || a->base.runtime.group_frame.candidate || a->base.runtime.panel_frame.candidate)installed=TRUE;
    else if(legacy_progress)installed=v->legacy_progress(w,NULL,a,&a->base.instance.detached);
    else if(protocol && style_control_protocol_toolbar(w))
        installed=v->toolbar(w,NULL,a,&a->base.instance.detached);
    else if(protocol && style_control_protocol_is(w,UPDOWN_CLASSW))installed=v->updown(w,NULL,a,&a->base.instance.detached);
    else if(protocol && style_control_protocol_is(w,TRACKBAR_CLASSW))installed=v->trackbar(w,NULL,a,&a->base.instance.detached);
    else if(protocol && style_control_protocol_is(w,PROGRESS_CLASSW))installed=v->progress(w,NULL,a,&a->base.instance.detached);
    else if(tree)installed=v->tree(w,NULL,a,&a->base.instance.detached);
    else if(picture)installed=v->picture(w,NULL,a,&a->base.instance.detached);
    else if(a->base.runtime.roles.references.popup.candidate)installed=v->grid(w,NULL,a,&a->base.instance.detached);
    else if(a->button_owner)installed=v->frame_scrollbars(w,NULL,a,&a->base.instance.detached);
    else if(control)installed=v->control(w,edit,(original_style&WS_BORDER)!=0,
        style_control_class_is(w,L"LISTBOX") && (original_style&LBS_USETABSTOPS) && !a->file_owner,NULL,a,&a->base.instance.detached);
    else if(report && !(original_style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION)))installed=TRUE;
    else installed=v->frame(w,NULL,a,&a->base.instance.detached);
    return installed;
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
       style_control_vcl_grid_identity(w,v->owner,GetParent(w))){v->control=FALSE;v->edit=FALSE;}
    return TRUE;
}

void win16_attachment_classify_vcl(HWND w,DWORD original_style,BOOL ready,BOOL picture,BOOL checklist,BOOL status,BOOL spin,
    const win16_attachment_initial_classification *i,style_control_runtime_candidates *v)
{
    *v=(style_control_runtime_candidates){0};
    if(!i->control && !i->report && !i->tree && !picture && !checklist && !i->legacy_progress && !status && !spin &&
       (original_style&WS_CHILD) && i->owner)style_control_vcl_classify(w,original_style,ready,i->owner,v);
}
int win16_attachment_eligibility(DWORD original_style,BOOL picture,BOOL checklist,BOOL status,BOOL spin,BOOL vcl_mdi_probe,
    const win16_attachment_initial_classification *i,const style_control_runtime_candidates *v,const style_control_runtime_extras *e)
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

BOOL win16_attachment_vcl_combo_candidate(HWND w,attachment *a)
{(void)w;return a && !a->base.instance.detached && a->base.runtime.combo;}



BOOL win16_attachment_group_push_button(HWND w,attachment *a)
{
    WCHAR name[64];
    return a && !a->base.instance.detached && a->base.runtime.group.candidate && a->base.runtime.group.installed &&
        a->base.runtime.group.parent==GetParent(w) && style_control_vcl_group_button_identity(w) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TButton");
}

BOOL win16_attachment_group_choice_button(HWND w,attachment *a)
{
    WCHAR name[64];
    return a && !a->base.instance.detached && a->base.runtime.group.candidate && a->base.runtime.group.installed &&
        a->base.runtime.group.parent==GetParent(w) && style_control_vcl_group_button_identity(w) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TCheckBox") || !lstrcmpW(name,L"TRadioButton") || !lstrcmpW(name,L"TGroupButton"));
}

BOOL win16_attachment_vb_data_valid(HWND w,attachment *a)
{

    return !a || a->base.instance.detached || !a->base.runtime.data.installed || style_control_vb_data_role(w,a->base.runtime.data.parent);
}

BOOL win16_attachment_vb_data_attached(HWND w,attachment *a)
{

    return a && !a->base.instance.detached && a->base.runtime.data.installed && style_control_vb_data_role(w,a->base.runtime.data.parent);
}

BOOL win16_attachment_vb_user_label_target(HWND w,attachment *a)
{

    if(!a || a->base.instance.detached || !a->base.runtime.user.candidate)return FALSE;
    HWND parent=GetParent(w);
    return style_control_vb_user_edge_role(w,parent) && (!a->base.runtime.user.parent || a->base.runtime.user.parent==parent);
}

BOOL win16_attachment_vb_user_edge_dormant(HWND w,attachment *a)
{

    return a && !a->base.instance.detached && a->base.runtime.user.installed && style_control_vb_user_edge_role(w,a->base.runtime.user.parent) &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE);
}

BOOL win16_attachment_vb_user_edge_valid(HWND w,attachment *a)
{

    return !a || a->base.instance.detached || !a->base.runtime.user.installed || style_control_vb_user_edge_identity(w,a->base.runtime.user.parent);
}




BOOL win16_attachment_inplace_paint(attachment *owner,HWND observed,HDC dc)
{return owner && owner->base.runtime.roles.inplace.candidate && !owner->base.instance.detached && dc && WindowFromDC(dc)==observed;}
