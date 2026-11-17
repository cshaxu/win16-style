#include "runtime_identity.h"
#include "../../window/window_identity.h"
#include <commctrl.h>
BOOL style_runtime_scrollbox_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64];
    DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return win32_style_window_owned_child(w,owner,parent) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TScrollBox") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) &&
        IsWindowUnicode(w) && (ex&WS_EX_CLIENTEDGE) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) &&
        win32_style_class_procedure_owned(owner,name);
}

BOOL style_runtime_controlbar_identity(HWND w,const style_instance *instance,const style_runtime_frame_state *a)
{
 WCHAR name[64],real[64];HWND parent=GetParent(w);DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
 return a && !instance->detached && !a->retired && IsWindow(w) && parent && parent==a->parent &&
  win32_style_window_current_thread(w) && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
  (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==a->owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==a->owner &&
  GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TControlBar") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) &&
  IsWindowUnicode(w) && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) &&
  win32_style_class_procedure_owned(a->owner,name);
}


BOOL style_runtime_rich_identity(HWND w,const style_instance *instance,const style_runtime_rich_state *a,DWORD rejected_month_styles)
{
    WCHAR name[64];HWND parent=GetParent(w);DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return a && !instance->detached && IsWindow(w) && parent && parent==a->parent &&
        GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() &&
        (a->popup?(parent==GetDesktopWindow() && a->owner==GetModuleHandleW(NULL) && (style&WS_BORDER) && !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))):(GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==a->owner)) &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==a->owner &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,a->popup?L"TPopupListbox":a->month?L"TMonthCalendar":a->hotkey?L"THotKey":a->unicode?L"TRichEdit":L"TRichEditURL") && (!a->unicode || IsWindowUnicode(w)) && (!a->month || !(style&rejected_month_styles)) && (style&WS_CHILD) &&
        (a->popup || a->month || (ex&WS_EX_CLIENTEDGE)) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED));
}

void style_runtime_frame_retire(HWND w,style_runtime_frame_state *state)
{
    if(state->candidate && state->parent && GetParent(w)!=state->parent){state->retired=TRUE;state->verified=FALSE;}
}
void style_runtime_box_settle(HWND w,style_instance *instance,style_runtime_frame_state *state,BOOL witnessed,const style_runtime_frame_ops *ops)
{
    if(witnessed && !state->verified && !instance->detached && style_runtime_scrollbox_identity(w,state->owner,state->parent)){
        DWORD error=GetLastError();state->verified=TRUE;
        if(ops->install(w,ops->context,&instance->detached) && !instance->detached && style_runtime_scrollbox_identity(w,state->owner,state->parent))RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
        else if(!instance->detached){state->verified=FALSE;ops->remove(w,ops->context);}
        SetLastError(error);
    }
}
void style_runtime_rich_settle(HWND w,UINT message,style_instance *instance,style_runtime_rich_state *state,DWORD rejected_month_styles,const style_runtime_frame_ops *ops)
{
    if(message!=WM_PAINT)return;
    if(state->candidate && !state->installed && state->witness && style_runtime_rich_identity(w,instance,state,rejected_month_styles)){
        DWORD error=GetLastError();state->installed=TRUE;
        if((state->month || ops->install(w,ops->context,&instance->detached)) && !instance->detached && style_runtime_rich_identity(w,instance,state,rejected_month_styles)){
            if(!state->month)RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
        }else if(!instance->detached){state->installed=FALSE;ops->remove(w,ops->context);}
        SetLastError(error);
    }
}
