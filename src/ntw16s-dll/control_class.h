#ifndef NTW16S_CONTROL_CLASS_H
#define NTW16S_CONTROL_CLASS_H
#include <windows.h>
#include "control_protocol.h"
/* Private framework classification. A name alone is insufficient: only the
 * exact observed intrinsic classes registered by their loaded VB5/6 runtime are aliases.
 * No LoadLibrary, object-layout inspection or application-name special case. */
static inline BOOL ntw16s_vb_class(HWND w,const WCHAR *name)
{
    WCHAR actual[64];HMODULE runtime=GetModuleHandleW(L"MSVBVM60.DLL");
    return runtime && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==runtime &&
        GetClassNameW(w,actual,ARRAYSIZE(actual)) && !lstrcmpiW(actual,name);
}
static inline BOOL ntw16s_vb5_class(HWND w,const WCHAR *name)
{
    WCHAR actual[64];HMODULE runtime=GetModuleHandleW(L"MSVBVM50.DLL");
    return runtime && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==runtime &&
        GetClassNameW(w,actual,ARRAYSIZE(actual)) && !lstrcmpiW(actual,name);
}
static inline BOOL ntw16s_vb_checkbox(HWND w)
{return ntw16s_vb_class(w,L"ThunderRT6CheckBox") || ntw16s_vb5_class(w,L"ThunderRT5CheckBox");}
static inline BOOL ntw16s_vb_option(HWND w)
{return ntw16s_vb_class(w,L"ThunderRT6OptionButton") || ntw16s_vb5_class(w,L"ThunderRT5OptionButton");}
/* The runtime retains Value/Click/group behavior, independent of style bits. */
static inline BOOL ntw16s_vb_button(HWND w)
{return ntw16s_vb_class(w,L"ThunderRT6CommandButton") || ntw16s_vb5_class(w,L"ThunderRT5CommandButton") || ntw16s_vb_checkbox(w) || ntw16s_vb_option(w);}
static inline BOOL ntw16s_vb_plain_choice(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=style&BS_TYPEMASK;
    if(style&(BS_PUSHLIKE|BS_ICON|BS_BITMAP))return FALSE;
    if(ntw16s_vb_option(w))return type==BS_RADIOBUTTON || type==BS_AUTORADIOBUTTON;
    return ntw16s_vb_checkbox(w) && (type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_3STATE || type==BS_AUTO3STATE);
}
/* USER identifies standard superclasses independently of the registered name.
 * Limit the fallback to the standard protocols implemented by this renderer;
 * arbitrary T-prefixed/custom classes are not aliases. */
static inline BOOL ntw16s_native_superclass(HWND w,const WCHAR *expected)
{
    WCHAR registered[64],real[64];
    return GetClassNameW(w,registered,ARRAYSIZE(registered)) && lstrcmpiW(registered,expected) &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpiW(real,expected);
}
static inline BOOL ntw16s_class_is(HWND w,const WCHAR *expected)
{
    WCHAR name[64];
    if(!lstrcmpiW(expected,L"SysTabControl32") || !lstrcmpiW(expected,L"SysHeader32") || !lstrcmpiW(expected,L"SysTreeView32") || !lstrcmpiW(expected,L"msctls_progress32") || !lstrcmpiW(expected,L"msctls_trackbar32") || !lstrcmpiW(expected,L"msctls_updown32"))
        return ntw16s_control_protocol_is(w,expected);
    if(GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpiW(name,expected))return TRUE;
    if((!lstrcmpiW(expected,L"BUTTON") || !lstrcmpiW(expected,L"EDIT") ||
        !lstrcmpiW(expected,L"STATIC") || !lstrcmpiW(expected,L"COMBOBOX") ||
        !lstrcmpiW(expected,L"LISTBOX") || !lstrcmpiW(expected,L"SCROLLBAR")) &&
        ntw16s_native_superclass(w,expected))return TRUE;
    if(!lstrcmpiW(expected,L"BUTTON") && (ntw16s_vb_button(w) || (ntw16s_vb_class(w,L"ThunderRT6Frame") || ntw16s_vb5_class(w,L"ThunderRT5Frame"))))return TRUE;
    if(!lstrcmpiW(expected,L"EDIT"))return ntw16s_vb_class(w,L"ThunderRT6TextBox") || ntw16s_vb5_class(w,L"ThunderRT5TextBox");
    return ntw16s_control_protocol_is(w,expected);
}
static inline BOOL ntw16s_vb_frame(HWND w)
{return ntw16s_vb_class(w,L"ThunderRT6Frame") || ntw16s_vb5_class(w,L"ThunderRT5Frame");}
static inline BOOL ntw16s_vb_edit(HWND w)
{return ntw16s_vb_class(w,L"ThunderRT6TextBox") || ntw16s_vb5_class(w,L"ThunderRT5TextBox");}
static inline UINT ntw16s_button_type(HWND w,DWORD style)
{return ntw16s_vb_frame(w)?BS_GROUPBOX:(style&BS_TYPEMASK);}
#endif
