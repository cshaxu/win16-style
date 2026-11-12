#ifndef NTW16S_CONTROL_CLASS_H
#define NTW16S_CONTROL_CLASS_H
#include <windows.h>
#include <commctrl.h>
#include "../common/control_protocol.h"
#include "../common/control_native_combo.h"
#include "../common/control_class.h"
/* Style-specific eligibility for native and VB controls. */

static inline BOOL ntw16s_vb_checklist_owner(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    UINT dpi=GetDpiForWindow(w);
    return (style&WS_CHILD) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        (dpi==96 || dpi==144) && style_control_vb_checklist_class(w) &&
        (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS|LBS_MULTIPLESEL|LBS_EXTENDEDSEL|LBS_NODATA))==
            (LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_MULTIPLESEL);
}
/* Only the observed intrinsic owner-draw modes; native runtime keeps images,
 * item data, measurement and input. Unsupported live styles stay native. */
static inline BOOL ntw16s_vb_owner_file(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    if(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING))return FALSE;
    if(ntw16s_vb_checklist_owner(w))return TRUE;
    if(style_control_vb_directory(w))return (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_HASSTRINGS))==(LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS);
    return style_control_vb_drive(w) && (style&(3|CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE|CBS_HASSTRINGS))==(CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS);
}

static inline BOOL ntw16s_vb_plain_choice(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=style&BS_TYPEMASK;
    if(style&(BS_PUSHLIKE|BS_ICON|BS_BITMAP))return FALSE;
    if(style_control_vb_option(w))return type==BS_RADIOBUTTON || type==BS_AUTORADIOBUTTON;
    return style_control_vb_checkbox(w) && (type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_3STATE || type==BS_AUTO3STATE);
}
/* VB graphical selection controls are not owner-drawn applications: the
 * runtime retains Value/Click/grouping while USER exposes its button face as
 * a push-like choice.  A bitmap/icon is safe only as the existing generic
 * BUTTON painter's borrowed BM_GETIMAGE payload; it never changes image
 * ownership.  Other owner-draw variants retain their native painter. */
static inline BOOL ntw16s_vb_graphical_choice(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=style&BS_TYPEMASK;
    if(!(style&BS_PUSHLIKE))return FALSE;
    if(style_control_vb_option(w))return type==BS_RADIOBUTTON || type==BS_AUTORADIOBUTTON;
    return style_control_vb_checkbox(w) &&
        (type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_3STATE || type==BS_AUTO3STATE);
}

static inline BOOL ntw16s_vb_picture(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return (style&WS_CHILD) && (style&WS_CAPTION)!=WS_CAPTION && !(style&WS_THICKFRAME) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        (style_control_vb_class(w,L"ThunderRT6PictureBoxDC") || style_control_vb5_class(w,L"ThunderRT5PictureBox"));
}
static inline BOOL ntw16s_vb_graphical_button(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return (style&WS_CHILD) && (style&BS_TYPEMASK)==BS_OWNERDRAW &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        (style_control_vb_class(w,L"ThunderRT6CommandButton") || style_control_vb5_class(w,L"ThunderRT5CommandButton") ||
         (style_control_vb_class(w,L"ThunderRT6OptionButton") || style_control_vb_class(w,L"ThunderRT6CheckBox") ||
          style_control_vb5_class(w,L"ThunderRT5CheckBox") || style_control_vb5_class(w,L"ThunderRT5OptionButton")));
}

static inline UINT ntw16s_button_type(HWND w,DWORD style)
{return style_control_vb_frame(w)?BS_GROUPBOX:(style&BS_TYPEMASK);}
#endif
