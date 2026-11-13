#include "control_vcl_identity.h"
#include "lib-win32/style/window_identity.h"
#include <commctrl.h>

/* Same-runtime VCL buttons witness the registered BUTTON procedure in paint. */
BOOL style_control_vcl_group_button_identity(HWND w)
{
    WCHAR name[64],real[64],parent_name[64];HWND parent=GetParent(w);
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    if(!(win32_style_window_owned_child(w,owner,parent) &&
        !(style&(BS_BITMAP|BS_ICON|BS_PUSHLIKE)) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real)))return FALSE;
    if(!lstrcmpW(name,L"TCheckBox"))return (style&BS_TYPEMASK)==BS_3STATE;
    if(!lstrcmpW(name,L"TButton"))
        return (style&BS_TYPEMASK)==BS_PUSHBUTTON || (style&BS_TYPEMASK)==BS_DEFPUSHBUTTON;
    return (style&BS_TYPEMASK)==BS_RADIOBUTTON &&
        (!lstrcmpW(name,L"TRadioButton") || (!lstrcmpW(name,L"TGroupButton") &&
         GetClassNameW(parent,parent_name,ARRAYSIZE(parent_name)) && !lstrcmpW(parent_name,L"TRadioGroup")));
}

BOOL style_control_vcl_grid_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64];DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return owner && parent && GetParent(w)==parent && win32_style_window_current_thread(w) &&
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

BOOL style_control_vcl_toolbar_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64],pn[64];DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return win32_style_window_owned_child(w,owner,parent) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TToolBar") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TToolBar") &&
        GetClassNameW(parent,pn,ARRAYSIZE(pn)) && !lstrcmpW(pn,L"TControlBar");
}

BOOL style_control_vcl_inplace_identity(HWND w,HMODULE owner,HWND parent)
{WCHAR name[64],real[64],pname[64];return win32_style_window_owned_child(w,owner,parent) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && IsWindowUnicode(w) && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TInplaceEditList") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name) && GetClassNameW(parent,pname,ARRAYSIZE(pname)) && !lstrcmpW(pname,L"TValueListEditor");}

BOOL style_control_vcl_lookup_identity(HWND w,HMODULE owner,HWND parent)
{WCHAR name[64],real[64];return win32_style_window_owned_child(w,owner,parent) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TDBLookupComboBox") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);}

BOOL style_control_vcl_inplace_or_lookup(HWND w,HMODULE owner,HWND parent)
{return style_control_vcl_inplace_identity(w,owner,parent) || style_control_vcl_lookup_identity(w,owner,parent);}

BOOL style_control_vcl_lookup_popup_identity(HWND w,HWND source,HWND source_parent,HMODULE owner)
{WCHAR name[64],real[64];DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);return w && source && owner && owner==GetModuleHandleW(NULL) && IsWindow(w) && style_control_vcl_lookup_identity(source,owner,source_parent) && win32_style_window_current_thread(w) && GetParent(w)==GetAncestor(source,GA_ROOT) && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (style&WS_POPUP) && (style&WS_BORDER) && !(style&WS_CHILD) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TPopupDataList") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real);}

BOOL style_control_vcl_nav_identity(HWND w,HMODULE owner,HWND parent)
{WCHAR name[64],real[64];return win32_style_window_owned_child(w,owner,parent) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TDBNavigator") || !lstrcmpW(name,L"TMediaPlayer")) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real);}

BOOL style_control_vcl_bit_identity(HWND w,HMODULE owner)
{
    WCHAR name[64],real[64];DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    return owner && win32_style_window_current_thread(w) &&
        (style&WS_CHILD) && (style&BS_TYPEMASK)==BS_OWNERDRAW &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TBitBtn") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TBitBtn");
}

/* This precise superclass owns every item and callback. */
BOOL style_control_vcl_checklist_identity(HWND w)
{
    WCHAR name[64],real[64];DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return win32_style_window_current_thread(w) &&
        GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==GetModuleHandleW(NULL) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"THintCheckListBox") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpiW(real,L"LISTBOX") &&
        (style&(WS_CHILD|LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_HASSTRINGS))==
            (WS_CHILD|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}
