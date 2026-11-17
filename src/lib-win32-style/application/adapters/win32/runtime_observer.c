#include "runtime_observer.h"
#include "../../control/control_native_witness.h"
#include "../../control/control_vcl_identity.h"

HMODULE style_runtime_observe_native(HWND w,UINT message,WNDPROC proc,const void *address,const style_runtime_observation *paint,style_runtime_observation *cool)
{
    WNDCLASSEXW native={.cbSize=sizeof(native)};HMODULE caller=NULL,native_owner=NULL,owner;
    if(message==WM_PAINT && cool && cool->window==w && !cool->instance->detached && cool->current(w,cool->owner) &&
       GetClassInfoExW(NULL,REBARCLASSNAMEW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) &&
       caller==cool->runtime->cool.owner)cool->native=proc;
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->status.candidate && !paint->instance->detached && !paint->runtime->status.attempted && paint->current(w,paint->owner)){
        owner=style_control_native_witness(w,proc,address,STATUSCLASSNAMEW,TRUE);
        if(owner)paint->runtime->status.witness_owner=owner;
    }
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->date.candidate && !paint->instance->detached && !paint->runtime->date.attempted && paint->current(w,paint->owner)){
        owner=style_control_native_witness(w,proc,address,DATETIMEPICK_CLASSW,TRUE);
        if(owner)paint->runtime->date.owner=owner;
    }
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->color.candidate && !paint->instance->detached && !paint->runtime->color.attempted && paint->current(w,paint->owner)){
        owner=style_control_native_witness(w,proc,address,L"COMBOBOX",TRUE);
        if(owner)paint->runtime->color.owner=owner;
    }
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->track.candidate && !paint->instance->detached && !paint->runtime->track.attempted && paint->current(w,paint->owner)){
        owner=style_control_native_witness(w,proc,address,TRACKBAR_CLASSW,TRUE);
        if(owner)paint->runtime->track.witness=owner;
    }
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->scroll.candidate && !paint->instance->detached &&
       !paint->runtime->scroll.attempted && paint->current(w,paint->owner) &&
       GetParent(w)==paint->runtime->scroll.parent){
        owner=style_control_native_witness(w,proc,address,L"SCROLLBAR",TRUE);
        if(owner)paint->runtime->scroll.witness=owner;
    }
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->edit.candidate && !paint->instance->detached &&
       !paint->runtime->edit.attempted && paint->current(w,paint->owner)){
        owner=style_control_native_witness(w,proc,address,paint->runtime->link?WC_LINK:paint->runtime->combo?L"COMBOBOX":L"EDIT",TRUE);
        if(owner){WCHAR name[64];
            if(!paint->runtime->combo || (GetClassNameW(w,name,ARRAYSIZE(name)) && lstrcmpW(name,L"TDBComboBox")))
                paint->runtime->edit.witness=owner;
        }
    }
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->group.candidate && !paint->instance->detached &&
       !paint->runtime->group.attempted && paint->current(w,paint->owner) &&
       style_control_vcl_group_button_identity(w)){
        owner=style_control_native_witness(w,proc,address,L"BUTTON",TRUE);
        if(owner)paint->runtime->group.witness=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PRINTCLIENT && paint && paint->window==w &&
       paint->runtime->toolbar.candidate && !paint->instance->detached &&
       !paint->runtime->toolbar.attempted && paint->current(w,paint->owner) &&
       style_control_vcl_toolbar_identity(w,paint->runtime->toolbar.owner,paint->runtime->toolbar.parent) &&
       GetClassInfoExW(NULL,TOOLBARCLASSNAMEW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) &&
       caller==paint->runtime->toolbar.owner)paint->runtime->toolbar.witness=caller;
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->tab.candidate && !paint->instance->detached &&
       !paint->runtime->tab.attempted && paint->current(w,paint->owner) &&
       GetParent(w)==paint->runtime->tab.parent){
        owner=style_control_native_witness(w,proc,address,WC_TABCONTROLW,TRUE);
        if(owner)paint->runtime->tab.witness=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PAINT && paint && paint->window==w &&
       paint->runtime->report.binding.candidate && !paint->instance->detached &&
       !paint->runtime->report.binding.attempted && paint->current(w,paint->owner) &&
       GetParent(w)==paint->runtime->report.binding.parent &&
       GetClassInfoExW(NULL,paint->runtime->report.tree_role?WC_TREEVIEWW:WC_LISTVIEWW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) &&
       caller==paint->runtime->report.binding.owner &&
       (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==caller)paint->runtime->report.binding.witness=caller;
    native.cbSize=sizeof(native);
    if(message!=WM_PAINT || !paint || paint->window!=w ||
       paint->instance->detached || !paint->current(w,paint->owner) ||
       !GetClassInfoExW(NULL,L"STATIC",&native) || proc!=native.lpfnWndProc ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&caller) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)(UINT_PTR)proc,&native_owner))return NULL;
    owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(!owner || caller!=owner || !native_owner)return NULL;
    return native_owner;
}

BOOL style_runtime_observe_box(HWND w,UINT msg,const void *address,const style_runtime_observation *paint)
{
    HMODULE caller=NULL;
    if(!paint || paint->window!=w || msg!=WM_NCPAINT || paint->instance->detached ||
       !style_runtime_scrollbox_identity(w,paint->runtime->box.owner,paint->runtime->box.parent) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) ||
       (caller!=paint->runtime->box.owner && caller!=GetModuleHandleW(L"user32.dll")))return FALSE;
    return TRUE;
}

BOOL style_runtime_observe_rich(WNDPROC proc,HWND w,UINT msg,const void *address,BOOL wide,const style_runtime_observation *paint,DWORD rejected_month_styles)
{
    WNDCLASSEXA c={.cbSize=sizeof(c)};WNDCLASSEXW cw={.cbSize=sizeof(cw)};HMODULE caller=NULL,rich;
    if(msg!=WM_PAINT || !paint || paint->window!=w || !paint->runtime->rich.candidate)return FALSE;
    rich=GetModuleHandleW(paint->runtime->rich.popup?L"user32.dll":(paint->runtime->rich.hotkey || paint->runtime->rich.month)?L"comctl32.dll":L"riched20.dll");
    if(!style_runtime_rich_identity(w,paint->instance,&paint->runtime->rich,rejected_month_styles) || wide!=paint->runtime->rich.unicode || !rich ||
       (wide?(!GetClassInfoExW(paint->runtime->rich.popup?NULL:rich,paint->runtime->rich.popup?L"LISTBOX":paint->runtime->rich.month?L"SysMonthCal32":paint->runtime->rich.hotkey?L"msctls_hotkey32":L"RichEdit20W",&cw) || proc!=cw.lpfnWndProc):(!GetClassInfoExA(rich,"RichEdit20A",&c) || proc!=c.lpfnWndProc)) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) ||
       caller!=paint->runtime->rich.owner)return FALSE;
    paint->runtime->rich.witness=TRUE;
    return TRUE;
}

void style_runtime_observe_ansi(const style_runtime_observation *paint,WNDPROC proc,HWND w,UINT message,const void *address,BOOL combo)
{
    HMODULE owner;WCHAR name[64];
    if(message!=WM_PAINT || !paint || paint->window!=w || !(combo?paint->runtime->combo:paint->runtime->edit.candidate) ||
        paint->instance->detached || paint->runtime->edit.attempted || !paint->current(w,paint->owner) ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpW(name,combo?L"TDBComboBox":L"TDBEdit"))return;
    owner=style_control_native_witness(w,proc,address,combo?"COMBOBOX":"EDIT",FALSE);
    if(owner)paint->runtime->edit.witness=owner;
}
