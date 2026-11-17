#include "report.h"
#include "control_scope.h"
void style_control_report_bind(HWND w,style_instance *instance,style_control_runtime *runtime,SUBCLASSPROC notify,void *owner)
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
    if(!instance->detached && module && parent &&
        SetWindowSubclass(parent,notify,(UINT_PTR)w,(DWORD_PTR)owner)){
        runtime->protocol=protocol;runtime->report.window=w;runtime->report.parent=parent;runtime->report.module=module;runtime->report.tree=tree;
    }else style_control_protocol_release(protocol);
}
void style_control_report_settle(HWND w,UINT message,style_instance *instance,style_control_runtime *runtime,const style_control_report_ops *ops)
{
    if(message!=WM_PAINT || !runtime->report.binding.candidate || runtime->report.binding.attempted || instance->detached || !runtime->report.binding.witness)return;
    DWORD saved=GetLastError();runtime->report.binding.attempted=TRUE;
    BOOL tree=runtime->report.tree_role;
    style_control_protocol *protocol=GetParent(w)==runtime->report.binding.parent?
        style_control_protocol_bind_verified_report(w,runtime->report.binding.witness,tree):NULL;
    if(protocol && !instance->detached){
        HMODULE module=tree?style_control_protocol_tree_module(w):style_control_protocol_list_module(w);
        HWND parent=GetParent(w);
        if(module && parent==runtime->report.binding.parent && SetWindowSubclass(parent,ops->notify,(UINT_PTR)w,(DWORD_PTR)ops->context)){
            runtime->protocol=protocol;runtime->report.window=w;runtime->report.parent=parent;runtime->report.module=module;runtime->report.tree=tree;
            BOOL installed=ops->install(w,tree,ops->context,&instance->detached);
            if(installed && !instance->detached){runtime->report.binding.installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!instance->detached){(void)ops->remove(w);}
        }else style_control_protocol_release(protocol);
    }else if(protocol)style_control_protocol_release(protocol);
    if(!instance->detached && !runtime->report.binding.installed){runtime->report.binding.attempted=FALSE;runtime->report.binding.witness=NULL;}
    SetLastError(saved);
}
void style_control_report_unbind(style_control_runtime *runtime,SUBCLASSPROC notify,const void *owner)
{
    style_control_subclass_remove_owned(runtime->report.parent,notify,(UINT_PTR)runtime->report.window,owner);
    runtime->report.parent=NULL;
}
