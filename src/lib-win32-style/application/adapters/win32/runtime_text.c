#include "runtime_text.h"
#include "../../control/report.h"
static _Thread_local style_runtime_text_scope *current,*native_current;
void style_runtime_text_begin(style_runtime_text_scope *scope,HWND w,UINT message,void *context,const style_runtime_text_ops *ops)
{
    *scope=(style_runtime_text_scope){0};
    if(message==WM_PAINT || message==WM_PRINT || message==WM_PRINTCLIENT || message==WM_ERASEBKGND){
        scope->native_previous=native_current;scope->native_window=w;scope->native_active=TRUE;native_current=scope;
    }
    if(!ops->include_text(w,message))return;
    scope->previous=current;scope->active=TRUE;scope->context=context;current=scope;
    const style_runtime_text_role role=ops->role(w,ops->context);scope->window=role.window;scope->label_only=role.label_only;
}
void style_runtime_text_end(style_runtime_text_scope *scope)
{
    if(scope->native_active)native_current=scope->native_previous;
    if(scope->active)current=scope->previous;
}
const style_runtime_text_scope *style_runtime_text_current(void){return current;}
BOOL style_runtime_text_native_target(HDC dc,HWND *w)
{*w=native_current?native_current->native_window:NULL;return dc && native_current && native_current->native_dc==dc;}
void style_runtime_text_report_enable(style_runtime_text_scope *scope,HWND w,HMODULE module,const BOOL *cancelled,BOOL (*label)(HWND))
{
    if(!scope->active || !module)return;
    scope->report=TRUE;scope->window=w;scope->report_module=module;scope->cancelled=cancelled;
    if(label(w))scope->label_only=TRUE;
}
void style_runtime_text_report_bind(HWND w,HDC dc)
{if(current && current->report && current->window==w)current->report_dc=dc;}
BOOL style_runtime_text_report_active(void){return current && current->report;}
BOOL style_runtime_text_report_target(HDC dc,HWND *w)
{
    *w=NULL;
    if(!current || !current->report || !dc || current->report_dc!=dc || (current->cancelled && *current->cancelled))return FALSE;
    *w=current->window;return *w!=NULL;
}
BOOL style_runtime_text_report_caller(HMODULE module)
{return current && current->report && (module==current->report_module || module==GetModuleHandleW(NULL));}
BOOL style_runtime_text_active(void)
{return current && current->window && (!current->cancelled || !*current->cancelled);}
BOOL style_runtime_text_direct_allowed(void){return !current || !current->label_only;}

BOOL style_runtime_text_caller(const void *address)
{
    HMODULE caller=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&caller))return FALSE;
    return style_runtime_text_report_caller(caller) || caller==GetModuleHandleW(L"MSVBVM60.DLL") || caller==GetModuleHandleW(L"MSVBVM50.DLL") || caller==GetModuleHandleW(L"USER32.DLL") ||
        caller==GetModuleHandleW(L"LPK.DLL") || caller==GetModuleHandleW(L"USP10.DLL") || caller==GetModuleHandleW(L"GDI32FULL.DLL");
}
BOOL style_runtime_text_label_caller(const void *address)
{
    if(style_runtime_text_direct_allowed())return TRUE;
    HMODULE caller=NULL;
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (const WCHAR *)address,&caller) && (caller==GetModuleHandleW(L"MSVBVM50.DLL") || caller==GetModuleHandleW(L"MSVBVM60.DLL"));
}


void style_runtime_text_prepare_report(style_runtime_text_scope *paint,HWND w,WPARAM p,style_instance *instance,
    style_control_runtime *runtime,BOOL file_owner,SUBCLASSPROC notify,void *owner,BOOL (*label)(HWND))
{
    if(paint->native_active)paint->native_dc=(HDC)p;
    if(paint->active && !instance->detached && !runtime->protocol && !runtime->report.binding.candidate)
        style_control_report_bind(w,instance,runtime,notify,owner);
    if(paint->active && runtime->report.module && !file_owner && !instance->detached){
        DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        if((runtime->report.tree || (((style&LVS_TYPEMASK)==LVS_REPORT || runtime->report.binding.candidate) &&
            !(style&(LVS_OWNERDRAWFIXED|LVS_OWNERDATA)) && !SendMessageW(w,LVM_ISGROUPVIEWENABLED,0,0) &&
            !instance->detached && (DWORD)GetWindowLongPtrW(w,GWL_STYLE)==style)) && !instance->detached)
            style_runtime_text_report_enable(paint,w,runtime->report.module,&instance->detached,label);
    }
}
