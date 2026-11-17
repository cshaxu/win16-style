#include "attachment.h"
#include "../../control/control_class.h"
#include "../../control/control_vcl_identity.h"
#include "../../control/report.h"
#include "../../control/control_vcl_spin.h"
BOOL style_runtime_attachment_publish(HWND w,style_runtime_attachment *a,SUBCLASSPROC proc,void (*discard)(void *))
{
    if(style_instance_attach(&a->instance,w,proc,NULL))return TRUE;
    DWORD error=GetLastError();
    style_control_protocol_release(a->runtime.protocol);
    style_runtime_references_release(&a->runtime.roles.references);
    discard(a->instance.context);SetLastError(error);return FALSE;
}
static style_runtime_attachment *lookup(HWND window,SUBCLASSPROC procedure)
{return (style_runtime_attachment *)style_instance_lookup(window,procedure);}
BOOL style_runtime_attachment_attached(HWND w,SUBCLASSPROC proc,const style_runtime_attachment_ops *v)
{
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    style_runtime_attachment *a=lookup(w,proc);
    return a
        && (!a->runtime.report.binding.candidate || (a->runtime.report.binding.installed && GetParent(w)==a->runtime.report.binding.parent))
        && (!a->runtime.cool.candidate || (a->runtime.cool.verified && v->cool_identity(w,a->instance.context)))
        && (!a->runtime.roles.references.mdi.candidate || style_runtime_reference_matches(w,&a->instance,&a->runtime.roles.references.mdi,TRUE,v->host_attached))
        && (!a->runtime.controlbar.candidate || (a->runtime.controlbar.verified && style_runtime_controlbar_identity(w,&a->instance,&a->runtime.controlbar)))
        && (!a->runtime.box.candidate || (a->runtime.box.verified && style_runtime_scrollbox_identity(w,a->runtime.box.owner,a->runtime.box.parent)))
        && (!a->runtime.spin.modern || a->runtime.spin.verified)
        && (!a->runtime.data.candidate || a->runtime.data.installed)
        && (!a->runtime.user.candidate || v->user_label(w))
        && (!a->runtime.roles.text.candidate || a->runtime.roles.text.verified)
        && (!a->runtime.scroll.candidate || (a->runtime.scroll.installed && GetParent(w)==a->runtime.scroll.parent))
        && (!a->runtime.edit.candidate || a->runtime.edit.installed)
        && (!a->runtime.group.candidate || a->runtime.group.installed)
        && (!a->runtime.grid.candidate || a->runtime.grid.installed)
        && (!a->runtime.toolbar.candidate || a->runtime.toolbar.installed)
        && (!a->runtime.track.candidate || a->runtime.track.installed)
        && (!a->runtime.tab.candidate || a->runtime.tab.installed)
        && (!a->runtime.color.candidate || a->runtime.color.installed)
        && (!a->runtime.date.candidate || a->runtime.date.installed)
        && (!a->runtime.status.candidate || a->runtime.status.installed)
        && (!a->runtime.rich.candidate || (a->runtime.rich.installed && style_runtime_rich_identity(w,&a->instance,&a->runtime.rich,v->month_rejection)))
        && (!a->runtime.roles.bit.candidate || a->runtime.roles.bit.verified)
        && (!a->runtime.roles.inplace.candidate || (a->runtime.roles.inplace.verified && style_control_vcl_inplace_or_lookup(w,a->runtime.roles.inplace.owner,a->runtime.roles.inplace.parent)))
        && (!a->runtime.roles.nav.candidate || (a->runtime.roles.nav.verified && style_control_vcl_nav_identity(w,a->runtime.roles.nav.owner,a->runtime.roles.nav.parent)))
        && (!a->runtime.roles.references.popup.candidate || style_runtime_reference_matches(w,&a->instance,&a->runtime.roles.references.popup,FALSE,v->host_attached))
        && (!a->runtime.group_frame.candidate || (a->runtime.group_frame.verified && !a->runtime.group_frame.retired && GetParent(w)==a->runtime.group_frame.parent))
        && (!a->runtime.panel_frame.candidate || (a->runtime.panel_frame.verified && !a->runtime.panel_frame.retired && GetParent(w)==a->runtime.panel_frame.parent));
}
void style_runtime_attachment_scroll_changed(HWND w,SUBCLASSPROC proc,const style_runtime_attachment_ops *v)
{
    if(GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() && lookup(w,proc)){
        if(style_control_class_is(w,L"SCROLLBAR"))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
        else v->scrollbars(w);
    }
}
BOOL style_runtime_attachment_remove(HWND w,SUBCLASSPROC proc,const style_runtime_attachment_ops *v)
{
    style_runtime_attachment *a;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    a=lookup(w,proc);if(!a){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_instance_retain(&a->instance);
    if(!style_instance_detach(&a->instance)){style_instance_release(&a->instance);return FALSE;}
    style_control_report_unbind(&a->runtime,v->report_notify,a->instance.context);style_control_notify_unbind(&a->runtime.cool.notify,v->cool_notify,a->instance.context);
    /* Existing owners handle reentrant destruction/removal independently. */
    v->frame_remove(w,a->instance.context);
    v->control_remove(w,a->instance.context);
    if(a->themed)v->theme_remove(a->themed,a->instance.context);
    v->theme_remove(w,NULL);
    style_control_runtime_dispose(&a->runtime);
    style_instance_release(&a->instance);return TRUE;
}
BOOL style_runtime_attachment_terminal(HWND w,SUBCLASSPROC proc,const style_runtime_attachment_ops *v)
{
    style_runtime_attachment *a;
    DWORD saved=GetLastError(),error=ERROR_SUCCESS;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    a=lookup(w,proc);
    if(a){
        style_instance_retain(&a->instance);
        if(!style_instance_detach(&a->instance))error=ERROR_INVALID_STATE;
    }
    /* These leaves only retire our exact nodes. They do not restore native
     * appearance, send application messages, or call a saved procedure. */
    if(!v->frame_terminal(w) && !error)error=GetLastError();
    if(!v->control_terminal(w) && !error)error=GetLastError();
    if(a && a->themed && a->themed!=w &&
            !v->theme_terminal(a->themed,a->instance.context) && !error)error=GetLastError();
    if(!v->theme_terminal(w,NULL) && !error)error=GetLastError();
    if(a){style_control_report_unbind(&a->runtime,v->report_notify,a->instance.context);style_control_notify_unbind(&a->runtime.cool.notify,v->cool_notify,a->instance.context);style_control_runtime_dispose(&a->runtime);style_instance_release(&a->instance);}
    SetLastError(error?error:saved);return !error;
}


void style_runtime_attachment_initialize(HWND w,DWORD original_style,style_runtime_attachment *a,const style_runtime_attachment_seed *seed)
{
    const style_control_runtime_candidates *v=seed->candidates;
    const style_control_runtime_extras *e=seed->extra;
    style_instance_retain(&a->instance);
    a->runtime.cool.candidate=e->cool_candidate;
    a->runtime.cool.notify.parent=GetParent(w);
    a->runtime.cool.owner=seed->owner;
    if(e->cool_candidate)a->runtime.cool.notify.window=w;
    HWND mdi_parent=GetParent(w),mdi_host=GetParent(mdi_parent);
    style_instance *mdi=seed->vcl_mdi_probe?style_instance_lookup(mdi_host,seed->procedure):NULL;
    style_runtime_reference_init(&a->runtime.roles.references.mdi,seed->vcl_mdi_probe,mdi_parent,mdi_host,seed->owner,mdi);
    a->runtime.roles.nav.candidate=v->nav_candidate;
    a->runtime.roles.nav.parent=GetParent(w);
    a->runtime.roles.nav.owner=seed->owner;
    style_instance *popup=seed->lookup_popup_candidate?seed->popup_source_instance:NULL;
    style_runtime_reference_init(&a->runtime.roles.references.popup,seed->lookup_popup_candidate,seed->popup_source_parent,seed->popup_source,seed->owner,popup);
    a->runtime.controlbar.candidate=e->controlbar_candidate;
    a->runtime.controlbar.parent=GetParent(w);
    a->runtime.controlbar.owner=seed->owner;
    a->runtime.rich.popup=e->rich_popup;
    a->runtime.rich.month=e->rich_month;
    a->runtime.rich.hotkey=e->rich_hotkey;
    a->runtime.box.candidate=e->box_candidate;
    a->runtime.box.owner=seed->owner;
    a->runtime.box.parent=GetParent(w);
    a->runtime.rich.unicode=e->rich_unicode;
    a->runtime.rich.candidate=e->rich_candidate;
    a->runtime.rich.parent=e->rich_popup?GetDesktopWindow():GetParent(w);
    a->runtime.rich.owner=seed->owner;
    a->runtime.group_frame.candidate=v->group_frame_candidate;
    a->runtime.panel_frame.candidate=v->panel_frame_candidate;
    a->runtime.user.candidate=e->vb_user_edge;
    a->runtime.data.candidate=e->vb_data_edge;
    a->runtime.protocol=seed->protocol;
    style_runtime_owner_roles roles=seed->owner_roles(w,seed->context);
    a->runtime.status.present=seed->status;
    a->runtime.spin.candidate=seed->spin;
    a->runtime.spin.modern=seed->spin&&style_control_vcl_modern_spin_shape(w);
    a->runtime.roles.text.candidate=v->static_candidate;
    a->runtime.scroll.candidate=v->scroll_candidate;
    a->runtime.scroll.parent=GetParent(w);
    a->runtime.edit.candidate=v->edit_candidate;
    a->runtime.link=v->edit_candidate && style_control_class_is(w,L"TLinkLabel");
    a->runtime.combo=v->edit_candidate && (style_control_class_is(w,L"TComboBox") || style_control_class_is(w,L"TDBComboBox"));
    a->runtime.combo_parent=GetParent(w);
    a->runtime.combo_type=original_style&3;
    a->runtime.spinedit=v->edit_candidate && style_control_class_is(w,L"TSpinEdit");
    a->runtime.group.candidate=v->group_button_candidate;
    a->runtime.grid.candidate=v->grid_candidate;
    a->runtime.roles.inplace.candidate=v->inplace_candidate;
    a->runtime.roles.inplace.owner=seed->owner;
    a->runtime.roles.inplace.parent=GetParent(w);
    a->runtime.toolbar.candidate=v->vcl_toolbar_candidate;
    a->runtime.toolbar.owner=seed->owner;
    a->runtime.toolbar.parent=GetParent(w);
    a->runtime.grid.owner=seed->owner;
    a->runtime.grid.parent=GetParent(w);
    a->runtime.edit_style=original_style;
    a->runtime.track.candidate=v->track_candidate;
    a->runtime.tab.candidate=v->tab_candidate;
    a->runtime.tab.parent=GetParent(w);
    a->runtime.report.binding.candidate=v->vcl_report_candidate;
    a->runtime.report.binding.parent=GetParent(w);
    a->runtime.report.binding.owner=seed->owner;
    a->runtime.report.tree_role=v->vcl_report_candidate && style_control_class_is(w,L"TTreeView");
    a->runtime.color.candidate=v->color_candidate;
    a->runtime.date.candidate=v->date_candidate;
    a->runtime.status.candidate=v->status_candidate;
    a->runtime.roles.bit.candidate=v->bit_candidate;
    if(seed->report || seed->tree || roles.file_owner || roles.button_owner || roles.checklist || a->runtime.roles.bit.candidate){
        a->runtime.report.window=w;
        a->runtime.report.tree=seed->tree;
        a->runtime.report.module=(roles.file_owner || roles.button_owner || a->runtime.roles.bit.candidate)?(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE):seed->tree?style_control_protocol_tree_module(w):style_control_protocol_list_module(w);
        a->runtime.report.parent=GetParent(w);
    }
}


BOOL style_runtime_attachment_install_finish(HWND w,style_runtime_attachment *a,DWORD original_style,BOOL control,BOOL edit,BOOL file_owner,BOOL installed,DWORD *out_error,const style_runtime_attachment_install_ops *v)
{
    DWORD error=GetLastError();
    if(installed && !a->instance.detached){
        if(control){
            HWND themed=NULL;BOOL list=FALSE;
            if(!file_owner && style_control_class_is(w,L"COMBOBOX")){
                COMBOBOXINFO info={.cbSize=sizeof(info)};
                if(v->get_combo_info(w,&info)){themed=info.hwndList;list=TRUE;}
            }else if(style_control_class_is(w,L"LISTBOX")){themed=w;list=TRUE;}
            else if(edit && (original_style&ES_MULTILINE))themed=w;
            if(themed){
                a->themed=themed;
                if(!v->theme_enable(themed,a->instance.context))installed=FALSE;
                else {
                    installed=list?v->list_scrollbar_enable(themed):v->edit_scrollbar_enable(themed);}
                error=GetLastError();
            }
        }
    }
    if(installed && !a->instance.detached && !a->runtime.data.candidate && !a->runtime.scroll.candidate && !a->runtime.edit.candidate && !a->runtime.group.candidate && !a->runtime.grid.candidate && !a->runtime.toolbar.candidate && !a->runtime.track.candidate && !a->runtime.tab.candidate && !a->runtime.report.binding.candidate && !a->runtime.color.candidate && !a->runtime.date.candidate && !a->runtime.status.candidate && !a->runtime.roles.bit.candidate && a->themed!=w)v->theme_remove(w,NULL);
    if(a->instance.detached){installed=FALSE;error=ERROR_CANCELLED;}
    if(!installed && !a->instance.detached)v->remove(w);
    *out_error=error;return installed;
}

BOOL style_runtime_attachment_current(HWND w,SUBCLASSPROC proc,const style_runtime_attachment_policy *policy)
{
    /* Each old guard performed its own lookup. A policy may retire/replace
     * the root, so never carry its pointer across the next guard boundary. */
    style_runtime_attachment *a=lookup(w,proc);
    if(a && !a->instance.detached &&
       !((!a->runtime.rich.candidate || !a->runtime.rich.installed || style_runtime_rich_identity(w,&a->instance,&a->runtime.rich,policy->month_rejection)) &&
         (!a->runtime.box.candidate || !a->runtime.box.verified || style_runtime_scrollbox_identity(w,a->runtime.box.owner,a->runtime.box.parent))))return FALSE;
    a=lookup(w,proc);
    if(a && !a->instance.detached && a->runtime.combo && a->runtime.edit.installed && !style_control_runtime_combo_current(w,&a->runtime))return FALSE;
    a=lookup(w,proc);
    if(a && !a->instance.detached && a->runtime.group.candidate && a->runtime.group.installed &&
       !(a->runtime.group.parent==GetParent(w) && style_control_vcl_group_button_identity(w)))return FALSE;
    a=lookup(w,proc);
    if(a && !a->instance.detached && a->runtime.grid.candidate && a->runtime.grid.installed &&
       !style_control_vcl_grid_identity(w,a->runtime.grid.owner,a->runtime.grid.parent))return FALSE;
    if(!policy->data_current(w) || !policy->user_current(w))return FALSE;
    a=lookup(w,proc);
    if(a && !a->instance.detached && a->runtime.toolbar.candidate && a->runtime.toolbar.installed &&
       !style_control_vcl_toolbar_identity(w,a->runtime.toolbar.owner,a->runtime.toolbar.parent))return FALSE;
    a=lookup(w,proc);
    if(a && !a->instance.detached && a->runtime.tab.candidate && a->runtime.tab.installed){
        WCHAR name[64];
        if(!(GetParent(w)==a->runtime.tab.parent && GetClassNameW(w,name,ARRAYSIZE(name)) &&
             (!lstrcmpW(name,L"TTabControl") || !lstrcmpW(name,L"TTabbedNotebook") || !lstrcmpW(name,L"TPageControl")) &&
             !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING))))return FALSE;
    }
    return TRUE;
}

BOOL style_runtime_attachment_control_install(HWND w,style_control_runtime_kind kind,void *context,const BOOL *cancelled)
{
    const style_runtime_attachment_controls *ops=context;style_runtime_attachment *a=ops->root;
    BOOL installed=ops->install(w,kind,a->instance.context,cancelled);
    if(kind==STYLE_RUNTIME_EDIT && installed && !a->instance.detached && !a->runtime.link && !a->runtime.combo && !a->runtime.spinedit && (a->runtime.edit_style&ES_MULTILINE)){
        a->themed=w;installed=ops->theme_enable(w,a->instance.context) && ops->edit_scrollbars(w);
    }
    return installed;
}
BOOL style_runtime_attachment_control_remove(HWND w,style_control_runtime_kind kind,const void *context)
{
    const style_runtime_attachment_controls *ops=context;style_runtime_attachment *a=ops->root;
    BOOL removed=ops->remove(w,a->instance.context);
    if(kind==STYLE_RUNTIME_EDIT && a->themed){ops->theme_remove(w,a->instance.context);a->themed=NULL;}
    return removed;
}
