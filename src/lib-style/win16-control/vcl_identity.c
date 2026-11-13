#include "vcl_identity.h"
#include "lib-style/common/control_vcl_identity.h"
#include "control_class.h"
#include "lib-win32/style/window_identity.h"
#include <commctrl.h>

BOOL win16_control_vb_data_role(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return parent && GetParent(w)==parent && (style&(WS_CHILD|WS_BORDER))==(WS_CHILD|WS_BORDER) &&
        (style&WS_CAPTION)!=WS_CAPTION && !(style&(WS_THICKFRAME|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_TOOLWINDOW|WS_EX_LAYERED)) &&
        (GetDpiForWindow(w)==96 || GetDpiForWindow(w)==144) && style_control_vb_class(w,L"ThunderRT6Data");
}

BOOL win16_control_vb_user_edge_role(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return parent && GetParent(w)==parent && (style&WS_CHILD) &&
        (style&WS_CAPTION)!=WS_CAPTION && !(style&(WS_THICKFRAME|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_TOOLWINDOW|WS_EX_LAYERED)) &&
        (style_control_vb_class(w,L"ThunderRT6UserControlDC") || style_control_vb5_class(w,L"ThunderRT5UserControl"));
}

BOOL win16_control_vb_user_edge_identity(HWND w,HWND parent)
{return win16_control_vb_user_edge_role(w,parent) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE);}

BOOL win16_control_box_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64];
    DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return win32_style_window_owned_child(w,owner,parent) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TScrollBox") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) &&
        IsWindowUnicode(w) && (ex&WS_EX_CLIENTEDGE) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) &&
        win32_style_class_procedure_owned(owner,name);
}

BOOL win16_control_controlbar_identity(HWND w,const win16_control_controlbar_view *a)
{
 WCHAR name[64],real[64];HWND parent=GetParent(w);DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
 return a && !*a->detached && !*a->retired && IsWindow(w) && parent && parent==*a->parent &&
  win32_style_window_current_thread(w) && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
  (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==*a->owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==*a->owner &&
  GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TControlBar") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) &&
  IsWindowUnicode(w) && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) &&
  win32_style_class_procedure_owned(*a->owner,name);
}


BOOL win16_control_cool_class_owner(HMODULE owner)
{
    return owner && win32_style_class_procedure_owned(owner,L"TCoolBar");
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

void win16_control_vcl_classify_extra(HWND w,DWORD original_style,BOOL ready,HMODULE owner,win16_control_vcl_extra_candidates *v)
{
    v->cool_candidate=ready && (original_style&WS_CHILD) && style_control_class_is(w,L"TCoolBar") && win16_control_cool_class_owner(owner) && GetParent(w) && (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner;
    v->box_candidate=ready && win16_control_box_identity(w,owner,GetParent(w));
    v->controlbar_candidate=ready && style_control_class_is(w,L"TControlBar") && owner && GetParent(w) && (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner;
    v->rich_month=style_control_class_is(w,L"TMonthCalendar");
    v->rich_hotkey=style_control_class_is(w,L"THotKey");
    v->rich_popup=style_control_class_is(w,L"TPopupListbox");
    v->rich_unicode=v->rich_popup || v->rich_month || v->rich_hotkey || style_control_class_is(w,L"TRichEdit");
    v->rich_candidate=ready && (v->rich_unicode || style_control_class_is(w,L"TRichEditURL")) && owner && GetParent(w) &&
        (v->rich_popup?(owner==GetModuleHandleW(NULL) && style_control_vcl_inplace_identity(GetParent(w),owner,GetParent(GetParent(w))) && (original_style&WS_BORDER) && (original_style&WS_VSCROLL) && !(original_style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))):((HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner && (v->rich_month || (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE))));
    v->vb_user_edge=win16_control_vb_user_edge_role(w,GetParent(w));
    v->vb_data_edge=win16_control_vb_data_role(w,GetParent(w));
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
