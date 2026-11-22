#include "control_vcl_identity.h"
#include "control_class.h"
#include "runtime.h"
#include "lib-win32-style/application/window/window_identity.h"
#include <commctrl.h>

BOOL style_control_vcl_cool_identity(HWND w,HWND parent,HMODULE owner,BOOL (*current)(HWND,void *),void *context)
{
    DWORD pid=0,ppid=0;WNDCLASSEXW c={.cbSize=sizeof(c)};HMODULE proc=NULL;
    return parent && IsWindow(w) && IsWindow(parent) && current && current(w,context) &&
        GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,&ppid)==GetCurrentThreadId() && ppid==pid &&
        style_control_class_is(w,L"TCoolBar") && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner &&
        GetClassInfoExW(owner,L"TCoolBar",&c) && c.lpfnWndProc &&
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)c.lpfnWndProc,&proc) && proc==owner;
}

BOOL style_control_vcl_mdi_identity(HWND w,HWND parent,HWND host,HMODULE owner,BOOL (*host_attached)(HWND))
{
    DWORD pid=0,host_pid=0,client_pid=0;
    return owner && parent && host && IsWindow(w) && IsWindow(parent) && IsWindow(host) &&
        GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,&client_pid)==GetCurrentThreadId() && client_pid==pid &&
        GetWindowThreadProcessId(host,&host_pid)==GetCurrentThreadId() && host_pid==pid &&
        GetParent(w)==parent && GetParent(parent)==host && style_control_class_is(w,L"TForm") &&
        style_control_class_is(parent,L"MDIClient") && style_control_class_is(host,L"TForm") &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==GetModuleHandleW(L"user32.dll") &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        (HMODULE)GetClassLongPtrW(host,GCLP_HMODULE)==owner && host_attached && host_attached(host) &&
        (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_MDICHILD) &&
        (GetWindowLongPtrW(w,GWL_STYLE)&WS_CAPTION)==WS_CAPTION;
}

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

BOOL style_control_vb_data_role(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return parent && GetParent(w)==parent && (style&(WS_CHILD|WS_BORDER))==(WS_CHILD|WS_BORDER) &&
        (style&WS_CAPTION)!=WS_CAPTION && !(style&(WS_THICKFRAME|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_TOOLWINDOW|WS_EX_LAYERED)) &&
        (GetDpiForWindow(w)==96 || GetDpiForWindow(w)==144) && style_control_vb_class(w,L"ThunderRT6Data");
}

BOOL style_control_vb_user_edge_role(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return parent && GetParent(w)==parent && (style&WS_CHILD) &&
        (style&WS_CAPTION)!=WS_CAPTION && !(style&(WS_THICKFRAME|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_TOOLWINDOW|WS_EX_LAYERED)) &&
        (style_control_vb_class(w,L"ThunderRT6UserControlDC") || style_control_vb5_class(w,L"ThunderRT5UserControl"));
}

BOOL style_control_vb_user_edge_identity(HWND w,HWND parent)
{return style_control_vb_user_edge_role(w,parent) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE);}


void style_control_vcl_classify(HWND w,DWORD original_style,BOOL ready,HMODULE owner,style_control_runtime_candidates *v)
{
    WCHAR name[64],real[64];DWORD type=original_style&SS_TYPEMASK;
    v->static_candidate=GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TStaticText") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TStaticText") &&
        (type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP) &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
    v->scroll_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TScrollBar") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TScrollBar");
    v->edit_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TEdit") || !lstrcmpW(name,L"TMemo") || !lstrcmpW(name,L"TMaskEdit") || !lstrcmpW(name,L"TDBEdit") || !lstrcmpW(name,L"TLabeledEdit") || !lstrcmpW(name,L"TNumberBox") || !lstrcmpW(name,L"TLinkLabel") || !lstrcmpW(name,L"TSpinEdit") || !lstrcmpW(name,L"TComboBox") || !lstrcmpW(name,L"TDBComboBox")) &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) && ((lstrcmpW(name,L"TComboBox") && lstrcmpW(name,L"TDBComboBox")) || style_control_vcl_plain_combo(w,GetParent(w)));
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
    v->bit_candidate=ready && style_control_vcl_bit_identity(w,owner);
    v->inplace_candidate=ready && style_control_vcl_inplace_or_lookup(w,owner,GetParent(w));
    v->nav_candidate=ready && (owner==GetModuleHandleW(NULL) || style_control_class_is(w,L"TMediaPlayer")) && style_control_vcl_nav_identity(w,owner,GetParent(w));
    v->group_frame_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TGroupBox") || !lstrcmpW(name,L"TRadioGroup")) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);
    v->panel_frame_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TPanel") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);
    v->group_button_candidate=ready && style_control_vcl_group_button_identity(w);
    v->grid_candidate=ready && style_control_vcl_grid_identity(w,owner,GetParent(w));
    v->vcl_toolbar_candidate=ready && style_control_vcl_toolbar_identity(w,owner,GetParent(w));
    v->status_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TStatusBar") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TStatusBar");
    v->date_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TDateTimePicker") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TDateTimePicker");
}

void style_control_vcl_classify_extra(HWND w,DWORD original_style,BOOL ready,HMODULE owner,style_control_runtime_extras *v)
{
    v->cool_candidate=ready && (original_style&WS_CHILD) && style_control_class_is(w,L"TCoolBar") && (owner && win32_style_class_procedure_owned(owner,L"TCoolBar")) && GetParent(w) && (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner;
    v->box_candidate=ready && style_runtime_scrollbox_identity(w,owner,GetParent(w));
    v->controlbar_candidate=ready && style_control_class_is(w,L"TControlBar") && owner && GetParent(w) && (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner;
    v->rich_month=style_control_class_is(w,L"TMonthCalendar");
    v->rich_hotkey=style_control_class_is(w,L"THotKey");
    v->rich_popup=style_control_class_is(w,L"TPopupListbox");
    v->rich_unicode=v->rich_popup || v->rich_month || v->rich_hotkey || style_control_class_is(w,L"TRichEdit");
    v->rich_candidate=ready && (v->rich_unicode || style_control_class_is(w,L"TRichEditURL")) && owner && GetParent(w) &&
        (v->rich_popup?(owner==GetModuleHandleW(NULL) && style_control_vcl_inplace_identity(GetParent(w),owner,GetParent(GetParent(w))) && (original_style&WS_BORDER) && (original_style&WS_VSCROLL) && !(original_style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))):((HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner && (v->rich_month || (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE))));
    v->vb_user_edge=style_control_vb_user_edge_role(w,GetParent(w));
    v->vb_data_edge=style_control_vb_data_role(w,GetParent(w));
    /* An external OCX can create itself with frame bits during construction
     * and then become a custom client surface.  Without an explicitly
     * verified public protocol it must stay native; only an application-owned
     * generic child frame may use the ordinary frame path. */
    v->vb_mdi_child=(original_style&WS_CAPTION)==WS_CAPTION &&
        (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_MDICHILD) &&
        (style_control_vb5_class(w,L"ThunderRT5Form") || style_control_vb_class(w,L"ThunderRT6FormDC")) &&
        style_control_class_is(GetParent(w),L"MDIClient") &&
        (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==GetModuleHandleW(L"user32.dll");
}
