#include "message.h"
#include "../../control/control_class.h"
#include "../../control/control_vcl_identity.h"
/* The synchronous call owns these objects. Nested messages borrow the prior
 * manager heads; the canonical dispatch receives these exact cleanup objects. */
typedef struct runtime_message {
    style_control_native_overlay overlay;
    style_control_status_scope status,*status_prior;
    style_control_spin_scope spin,*spin_prior;
    style_runtime_text_scope text;
    BOOL spin_message,box_witnessed;
} runtime_message;
LRESULT style_runtime_message_dispatch(style_runtime_paint_manager *manager,const style_runtime_observation *observation,
    UINT m,WPARAM p,LPARAM l,const style_runtime_message_ops *ops)
{
    HWND w=observation->window;style_instance *instance=observation->instance;style_control_runtime *r=observation->runtime;
    DWORD error=GetLastError();BOOL preserve=r->status.present || r->spin.candidate || r->color.candidate || r->date.candidate;
    runtime_message call={0};
    style_runtime_frame_retire(w,&r->controlbar);
    style_runtime_frame_retire(w,&r->panel_frame);
    style_runtime_frame_retire(w,&r->group_frame);
    style_control_native_overlay_begin(w,m,p,instance,&r->color,&r->date,&call.overlay);
    style_instance_retain(instance);
    ops->guard(observation,m);
    call.status_prior=manager->status;call.spin_prior=manager->spin;
    call.spin_message=style_control_spin_message(instance,&r->spin,m);
    if(call.spin_prior)manager->spin=NULL;
    if(call.spin_message){
        call.spin.state=&r->spin;call.spin.instance=instance;call.spin.window=w;call.spin.parent=GetParent(w);
        call.spin.dc=(m==WM_PRINT || m==WM_PRINTCLIENT)?(HDC)p:NULL;manager->spin=&call.spin;
        call.spin.previous_stack=manager->spin_stack;manager->spin_stack=&call.spin;
    }
    BOOL painting=r->status.present && !instance->detached && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);
    if(call.status_prior && call.status_prior->window!=w)manager->status=NULL;
    if(painting){
        call.status=(style_control_status_scope){.previous=manager->status,.context=observation->owner,.window=w,.dc=m==WM_PAINT?NULL:(HDC)p,
            .instance=instance,.state=&r->status,.class_name=ops->status_class};
        style_control_status_read(w,instance,&r->status,ops->status_class,&call.status.snapshot);
        manager->status=&call.status;
    }
    ops->prepare(observation,m,p,&call.text);
    if(m==WM_NCPAINT && r->controlbar.candidate)r->controlbar.verified=FALSE;
    if(m==WM_PAINT && r->rich.candidate && (!r->rich.installed || r->rich.month))r->rich.witness=FALSE;
    if(preserve)SetLastError(error);
    const style_runtime_paint_cleanup cleanup={&call.text,call.status_prior,call.spin_prior,&call.spin,&call.overlay,&call.box_witnessed,NULL};
    const style_runtime_dispatch kind=style_runtime_dispatch_kind(w,instance,r,m,p,l,manager->status==&call.status,
        call.overlay.clip.region!=NULL,call.spin_message,manager->paint!=NULL);
    LRESULT result=style_runtime_paint_dispatch(manager,observation,kind,w,m,p,l,&cleanup);
    if(preserve)error=GetLastError();
    style_runtime_text_end(&call.text);manager->status=call.status_prior;
    if(call.spin_message){manager->spin_stack=call.spin.previous_stack;style_control_spin_buffers_clear(&call.spin);}
    manager->spin=call.spin_prior;
    style_runtime_box_settle(w,instance,&r->box,call.box_witnessed,&ops->frames);
    style_control_runtime_settle(w,m,instance,r,STYLE_RUNTIME_TOOLBAR,&ops->controls);
    style_control_edge_settle(w,m,instance,&r->data,STYLE_EDGE_DATA,&ops->edges);
    style_control_edge_settle(w,m,instance,&r->user,STYLE_EDGE_USER,&ops->edges);
    style_runtime_rich_settle(w,m,instance,&r->rich,ops->month_rejection,&ops->frames);
    style_control_edge_settle(w,m,instance,&r->grid,STYLE_EDGE_GRID,&ops->edges);
    style_control_runtime_settle(w,m,instance,r,STYLE_RUNTIME_GROUP,&ops->controls);
    style_control_runtime_settle(w,m,instance,r,STYLE_RUNTIME_SCROLL,&ops->controls);
    style_control_runtime_settle(w,m,instance,r,STYLE_RUNTIME_EDIT,&ops->controls);
    if(m==WM_PAINT)style_control_report_settle(w,WM_PAINT,instance,r,&ops->report);
    style_control_runtime_settle(w,m,instance,r,STYLE_RUNTIME_TAB,&ops->controls);
    style_control_runtime_settle(w,m,instance,r,STYLE_RUNTIME_TRACK,&ops->controls);
    style_control_native_combo_settle(w,m,instance,&r->color,FALSE);
    style_control_status_settle(w,m,instance,&r->status,ops->status_class);
    style_control_native_combo_settle(w,m,instance,&r->date,TRUE);
    style_control_native_overlay_finish(w,m,p,instance,&r->color,&r->date,&call.overlay,ops->button);
    style_instance_release(instance);if(preserve)SetLastError(error);return result;
}

LRESULT style_runtime_report_notify(style_runtime_paint_manager *manager,const style_runtime_observation *observation,HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,const style_runtime_report_notify_ops *v)
{
    DWORD entry=GetLastError();style_instance *instance=observation->instance;style_control_runtime *runtime=observation->runtime;BOOL draw=FALSE;HWND list=runtime->report.window;HDC dc=NULL;DWORD stage=0;
    style_runtime_text_scope file_paint;BOOL file_active=FALSE;
    style_runtime_checklist_scope checklist_scope;BOOL checklist_active=FALSE;
    style_instance_retain(instance);
    if(runtime->roles.bit.candidate && !instance->detached && m==WM_DRAWITEM && l && GetParent(list)==w && style_control_vcl_bit_identity(list,runtime->report.module)){
        const DRAWITEMSTRUCT *item=(const DRAWITEMSTRUCT *)l;
        if(item->CtlType==ODT_BUTTON && item->itemAction==ODA_DRAWENTIRE && item->hwndItem==list && item->hDC){
            HWND target=WindowFromDC(item->hDC);
            if(!target || target==list){
                const style_runtime_observation bit={.instance=instance,.runtime=runtime,.owner=observation->owner,.window=w};
                const style_runtime_paint_cleanup cleanup={.item=item};
                SetLastError(entry);LRESULT result=style_runtime_paint_dispatch(manager,&bit,STYLE_RUNTIME_DISPATCH_BIT,w,m,p,l,&cleanup);
                DWORD error=GetLastError();style_instance_release(instance);SetLastError(error);return result;
            }
        }
    }
    if(v->checklist){
        const style_runtime_observation checklist={.instance=instance,.runtime=runtime,.window=list};
        checklist_active=style_runtime_checklist_begin(&manager->checklists,&checklist_scope,&checklist,w,m,l,style_control_vcl_checklist_identity);
    }
    if((v->file_owner || v->button_owner) && !instance->detached && m==WM_DRAWITEM && l && GetParent(list)==w &&
            (v->button_owner?v->button_identity(list):v->file_identity(list))){
        DRAWITEMSTRUCT *item=(DRAWITEMSTRUCT *)l;
        if(item->hwndItem==list && item->hDC && item->CtlType==(v->button_owner?ODT_BUTTON:style_control_vb_drive(list)?ODT_COMBOBOX:ODT_LISTBOX)){
            HWND target=WindowFromDC(item->hDC);BOOL valid=!target || target==list;
            if(!valid && style_control_vb_drive(list)){
                COMBOBOXINFO combo={.cbSize=sizeof(combo)};
                valid=GetComboBoxInfo(list,&combo) && combo.hwndCombo==list && target==combo.hwndList &&
                    GetWindowThreadProcessId(target,NULL)==GetCurrentThreadId();
            }
            if(valid && !instance->detached){
                v->prepare_text(&file_paint,list,observation->owner);file_active=file_paint.active;
                if(file_active){HWND painted=target?target:list;
                    style_runtime_text_report_enable(&file_paint,painted,runtime->report.module,&instance->detached,v->button_identity);
                    style_runtime_text_report_bind(painted,item->hDC);
                }
            }
        }
    }
    if(!v->file_owner && !v->button_owner && !v->checklist && !runtime->roles.bit.candidate && m==WM_NOTIFY && l){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==list && header->code==NM_CUSTOMDRAW){
            NMCUSTOMDRAW *cd=(NMCUSTOMDRAW *)l;dc=cd->hdc;stage=cd->dwDrawStage;draw=TRUE;
            style_runtime_text_report_bind(list,NULL);
        }
    }
    if(m==WM_NCDESTROY){RemoveWindowSubclass(w,v->notify,id);runtime->report.parent=NULL;}
    SetLastError(entry);LRESULT result=DefSubclassProc(w,m,p,l);DWORD error=GetLastError();
    if(checklist_active)style_runtime_checklist_end(&manager->checklists,&checklist_scope);
    if(file_active)style_runtime_text_end(&file_paint);
    if(draw && !instance->detached && !(result&CDRF_SKIPDEFAULT) &&
        (stage==CDDS_PREPAINT || stage==CDDS_ITEMPREPAINT || stage==(CDDS_ITEMPREPAINT|CDDS_SUBITEM)))
        style_runtime_text_report_bind(list,dc);
    style_instance_release(instance);SetLastError(error);return result;
}
