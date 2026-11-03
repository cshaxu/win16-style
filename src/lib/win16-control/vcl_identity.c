#include "vcl_identity.h"
#include "control_class.h"
#include <commctrl.h>

/* Editable VCL combos are bound only to the original same-runtime parent. */
BOOL win16_control_combo_identity(HWND w,HWND parent)
{
    WCHAR name[64],real[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    return owner && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (style&WS_CHILD) && ((style&3)==CBS_SIMPLE || (style&3)==CBS_DROPDOWN || (style&3)==CBS_DROPDOWNLIST) && !(style&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE)) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TComboBox") || (!lstrcmpW(name,L"TDBComboBox") && (style&3)==CBS_DROPDOWN)) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && (!lstrcmpW(name,real) || !lstrcmpiW(real,L"COMBOBOX"));
}

/* Same-runtime VCL buttons witness the registered BUTTON procedure in paint. */
BOOL win16_control_group_button_identity(HWND w)
{
    WCHAR name[64],real[64],parent_name[64];HWND parent=GetParent(w);
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),pid=0;
    if(!(owner && parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (style&WS_CHILD) &&
        !(style&(BS_BITMAP|BS_ICON|BS_PUSHLIKE)) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real)))return FALSE;
    if(!lstrcmpW(name,L"TCheckBox"))return (style&BS_TYPEMASK)==BS_3STATE;
    if(!lstrcmpW(name,L"TButton"))
        return (style&BS_TYPEMASK)==BS_PUSHBUTTON || (style&BS_TYPEMASK)==BS_DEFPUSHBUTTON;
    return (style&BS_TYPEMASK)==BS_RADIOBUTTON &&
        (!lstrcmpW(name,L"TRadioButton") || (!lstrcmpW(name,L"TGroupButton") &&
         GetClassNameW(parent,parent_name,ARRAYSIZE(parent_name)) && !lstrcmpW(parent_name,L"TRadioGroup")));
}

BOOL win16_control_grid_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return owner && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        (style&WS_CHILD) && (ex&WS_EX_CLIENTEDGE) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) &&
        (((!lstrcmpW(name,L"TStringGrid") || !lstrcmpW(name,L"TDrawGrid") ||
          (IsWindowUnicode(w) && !lstrcmpW(name,L"TCategoryButtons") && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner) ||
          (owner==GetModuleHandleW(NULL) && IsWindowUnicode(w) && (!lstrcmpW(name,L"TValueListEditor") || !lstrcmpW(name,L"TDBGrid") ||
           ((!lstrcmpW(name,L"TDBLookupListBox") || !lstrcmpW(name,L"TOutline") || !lstrcmpW(name,L"TButtonGroup") || !lstrcmpW(name,L"TCategoryButtons")) && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner)))) && !lstrcmpW(name,real)) ||
         (owner==GetModuleHandleW(NULL) && IsWindowUnicode(w) && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner &&
          (!lstrcmpW(name,L"TDirectoryListBox") || !lstrcmpW(name,L"TFileListBox")) && !lstrcmpiW(real,L"LISTBOX") &&
          (style&(WS_HSCROLL|WS_VSCROLL)) && (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE)) && (style&LBS_HASSTRINGS) && !(style&LBS_NODATA)));
}

BOOL win16_control_vcl_toolbar_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64],pn[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return owner && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (style&WS_CHILD) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TToolBar") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TToolBar") &&
        GetClassNameW(parent,pn,ARRAYSIZE(pn)) && !lstrcmpW(pn,L"TControlBar");
}

BOOL win16_control_inplace_identity(HWND w,HMODULE owner,HWND parent)
{WCHAR name[64],real[64],pname[64];DWORD pid=0;return w && parent && owner && IsWindow(w) && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && IsWindowUnicode(w) && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TInplaceEditList") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name) && GetClassNameW(parent,pname,ARRAYSIZE(pname)) && !lstrcmpW(pname,L"TValueListEditor");}

BOOL win16_control_lookup_identity(HWND w,HMODULE owner,HWND parent)
{WCHAR name[64],real[64];DWORD pid=0;return w && parent && owner && IsWindow(w) && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TDBLookupComboBox") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);}

BOOL win16_control_inplace_or_lookup(HWND w,HMODULE owner,HWND parent)
{return win16_control_inplace_identity(w,owner,parent) || win16_control_lookup_identity(w,owner,parent);}

BOOL win16_control_lookup_popup_identity(HWND w,HWND source,HWND source_parent,HMODULE owner)
{WCHAR name[64],real[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);return w && source && owner && owner==GetModuleHandleW(NULL) && IsWindow(w) && win16_control_lookup_identity(source,owner,source_parent) && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetParent(w)==GetAncestor(source,GA_ROOT) && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (style&WS_POPUP) && (style&WS_BORDER) && !(style&WS_CHILD) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TPopupDataList") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real);}

BOOL win16_control_nav_identity(HWND w,HMODULE owner,HWND parent)
{WCHAR name[64],real[64];DWORD pid=0;return w && parent && owner && IsWindow(w) && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TDBNavigator") || !lstrcmpW(name,L"TMediaPlayer")) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real);}

BOOL win16_control_bit_identity(HWND w,HMODULE owner)
{
    WCHAR name[64],real[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    return owner && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        (style&WS_CHILD) && (style&BS_TYPEMASK)==BS_OWNERDRAW &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TBitBtn") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TBitBtn");
}

/* This precise superclass owns every item and callback. */
BOOL win16_control_checklist_identity(HWND w)
{
    WCHAR name[64],real[64];DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE),pid=0;
    return GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==GetModuleHandleW(NULL) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"THintCheckListBox") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpiW(real,L"LISTBOX") &&
        (style&(WS_CHILD|LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_HASSTRINGS))==
            (WS_CHILD|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

BOOL win16_control_vb_data_role(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return parent && GetParent(w)==parent && (style&(WS_CHILD|WS_BORDER))==(WS_CHILD|WS_BORDER) &&
        (style&WS_CAPTION)!=WS_CAPTION && !(style&(WS_THICKFRAME|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_TOOLWINDOW|WS_EX_LAYERED)) &&
        (GetDpiForWindow(w)==96 || GetDpiForWindow(w)==144) && ntw16s_vb_class(w,L"ThunderRT6Data");
}

BOOL win16_control_vb_user_edge_role(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return parent && GetParent(w)==parent && (style&WS_CHILD) &&
        (style&WS_CAPTION)!=WS_CAPTION && !(style&(WS_THICKFRAME|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_TOOLWINDOW|WS_EX_LAYERED)) &&
        (ntw16s_vb_class(w,L"ThunderRT6UserControlDC") || ntw16s_vb5_class(w,L"ThunderRT5UserControl"));
}

BOOL win16_control_vb_user_edge_identity(HWND w,HWND parent)
{return win16_control_vb_user_edge_role(w,parent) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE);}

BOOL win16_control_box_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64];WNDCLASSEXW c={.cbSize=sizeof(c)};HMODULE proc_owner=NULL;
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return owner && parent && IsWindow(w) && GetParent(w)==parent &&
        GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TScrollBox") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) &&
        IsWindowUnicode(w) && (style&WS_CHILD) && (ex&WS_EX_CLIENTEDGE) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) &&
        GetClassInfoExW(owner,name,&c) && c.lpfnWndProc &&
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)c.lpfnWndProc,&proc_owner) && proc_owner==owner;
}

BOOL win16_control_controlbar_identity(HWND w,const win16_control_controlbar_view *a)
{
 WCHAR name[64],real[64];DWORD pid=0;WNDCLASSEXW c={.cbSize=sizeof(c)};HMODULE proc_owner=NULL;HWND parent=GetParent(w);DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
 return a && !*a->detached && !*a->retired && IsWindow(w) && parent && parent==*a->parent &&
  GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
  (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==*a->owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==*a->owner &&
  GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TControlBar") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) &&
  IsWindowUnicode(w) && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) &&
  GetClassInfoExW(*a->owner,name,&c) && c.lpfnWndProc && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)c.lpfnWndProc,&proc_owner) && proc_owner==*a->owner;
}


BOOL win16_control_cool_class_owner(HMODULE owner)
{
    WNDCLASSEXW c={.cbSize=sizeof(c)};HMODULE proc=NULL;
    return owner && GetClassInfoExW(owner,L"TCoolBar",&c) && c.lpfnWndProc &&
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)c.lpfnWndProc,&proc) && proc==owner;
}

BOOL win16_control_rich_identity(HWND w,const win16_control_rich_view *a)
{
    WCHAR name[64];HWND parent=GetParent(w);DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return a && !*a->detached && IsWindow(w) && parent && parent==*a->parent &&
        GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() &&
        (*a->popup?(parent==GetDesktopWindow() && *a->owner==GetModuleHandleW(NULL) && (style&WS_BORDER) && !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))):(GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==*a->owner)) &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==*a->owner &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,*a->popup?L"TPopupListbox":*a->month?L"TMonthCalendar":*a->hotkey?L"THotKey":*a->unicode?L"TRichEdit":L"TRichEditURL") && (!*a->unicode || IsWindowUnicode(w)) && (!*a->month || !(style&MCS_MULTISELECT)) && (style&WS_CHILD) &&
        (*a->popup || *a->month || (ex&WS_EX_CLIENTEDGE)) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED));
}
