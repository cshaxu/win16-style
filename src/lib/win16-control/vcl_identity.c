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

void win16_control_vcl_classify(HWND w,DWORD original_style,BOOL ready,HMODULE owner,win16_control_vcl_candidates *v)
{
    WCHAR name[64],real[64];DWORD type=original_style&SS_TYPEMASK;
    v->static_candidate=GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TStaticText") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TStaticText") &&
        (type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP) &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
    v->scroll_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TScrollBar") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TScrollBar");
    v->edit_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TEdit") || !lstrcmpW(name,L"TMemo") || !lstrcmpW(name,L"TMaskEdit") || !lstrcmpW(name,L"TDBEdit") || !lstrcmpW(name,L"TLabeledEdit") || !lstrcmpW(name,L"TNumberBox") || !lstrcmpW(name,L"TLinkLabel") || !lstrcmpW(name,L"TSpinEdit") || !lstrcmpW(name,L"TComboBox") || !lstrcmpW(name,L"TDBComboBox")) &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) && ((lstrcmpW(name,L"TComboBox") && lstrcmpW(name,L"TDBComboBox")) || win16_control_combo_identity(w,GetParent(w)));
    v->tab_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TTabControl") || !lstrcmpW(name,L"TTabbedNotebook") || !lstrcmpW(name,L"TPageControl")) &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name) &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
    v->vcl_report_candidate=ready && owner!=GetModuleHandleW(NULL) && GetClassNameW(w,name,ARRAYSIZE(name)) &&
        (!lstrcmpW(name,L"TTreeView") || !lstrcmpW(name,L"TListView")) &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name) &&
        GetParent(w) && GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
    v->track_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TTrackBar") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TTrackBar");
    v->color_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) &&
        (!lstrcmpW(name,L"TColorBox") || !lstrcmpW(name,L"TDriveComboBox")) &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);
    v->bit_candidate=ready && win16_control_bit_identity(w,owner);
    v->inplace_candidate=ready && win16_control_inplace_or_lookup(w,owner,GetParent(w));
    v->nav_candidate=ready && (owner==GetModuleHandleW(NULL) || ntw16s_class_is(w,L"TMediaPlayer")) && win16_control_nav_identity(w,owner,GetParent(w));
    v->group_frame_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TGroupBox") || !lstrcmpW(name,L"TRadioGroup")) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);
    v->panel_frame_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TPanel") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);
    v->group_button_candidate=ready && win16_control_group_button_identity(w);
    v->grid_candidate=ready && win16_control_grid_identity(w,owner,GetParent(w));
    v->vcl_toolbar_candidate=ready && win16_control_vcl_toolbar_identity(w,owner,GetParent(w));
    v->status_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TStatusBar") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TStatusBar");
    v->date_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TDateTimePicker") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TDateTimePicker");
}

void win16_control_vcl_classify_extra(HWND w,DWORD original_style,BOOL ready,HMODULE owner,win16_control_vcl_extra_candidates *v)
{
    v->cool_candidate=ready && (original_style&WS_CHILD) && ntw16s_class_is(w,L"TCoolBar") && win16_control_cool_class_owner(owner) && GetParent(w) && (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner;
    v->box_candidate=ready && win16_control_box_identity(w,owner,GetParent(w));
    v->controlbar_candidate=ready && ntw16s_class_is(w,L"TControlBar") && owner && GetParent(w) && (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner;
    v->rich_month=ntw16s_class_is(w,L"TMonthCalendar");
    v->rich_hotkey=ntw16s_class_is(w,L"THotKey");
    v->rich_popup=ntw16s_class_is(w,L"TPopupListbox");
    v->rich_unicode=v->rich_popup || v->rich_month || v->rich_hotkey || ntw16s_class_is(w,L"TRichEdit");
    v->rich_candidate=ready && (v->rich_unicode || ntw16s_class_is(w,L"TRichEditURL")) && owner && GetParent(w) &&
        (v->rich_popup?(owner==GetModuleHandleW(NULL) && win16_control_inplace_identity(GetParent(w),owner,GetParent(GetParent(w))) && (original_style&WS_BORDER) && (original_style&WS_VSCROLL) && !(original_style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))):((HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner && (v->rich_month || (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE))));
    v->vb_user_edge=win16_control_vb_user_edge_role(w,GetParent(w));
    v->vb_data_edge=win16_control_vb_data_role(w,GetParent(w));
    /* An external OCX can create itself with frame bits during construction
     * and then become a custom client surface.  Without an explicitly
     * verified public protocol it must stay native; only an application-owned
     * generic child frame may use the ordinary frame path. */
    v->vb_mdi_child=(original_style&WS_CAPTION)==WS_CAPTION &&
        (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_MDICHILD) &&
        (ntw16s_vb5_class(w,L"ThunderRT5Form") || ntw16s_vb_class(w,L"ThunderRT6FormDC")) &&
        ntw16s_class_is(GetParent(w),L"MDIClient") &&
        (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==GetModuleHandleW(L"user32.dll");
}
