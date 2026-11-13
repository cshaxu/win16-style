#ifndef STYLE_CONTROL_CLASS_H
#define STYLE_CONTROL_CLASS_H
#include <windows.h>
#include <commctrl.h>
#include "control_protocol.h"
#include "control_native_combo.h"

/* Intrinsic Windows and VB runtime class identity; no style drawing policy. */
static inline BOOL style_control_vb_class(HWND w,const WCHAR *name)
{return style_control_runtime_class(w,name,L"MSVBVM60.DLL");}
static inline BOOL style_control_vb5_class(HWND w,const WCHAR *name)
{return style_control_runtime_class(w,name,L"MSVBVM50.DLL");}
static inline BOOL style_control_vb_checkbox(HWND w)
{return style_control_vb_class(w,L"ThunderRT6CheckBox") || style_control_vb5_class(w,L"ThunderRT5CheckBox");}
static inline BOOL style_control_vb_option(HWND w)
{return style_control_vb_class(w,L"ThunderRT6OptionButton") || style_control_vb5_class(w,L"ThunderRT5OptionButton");}
static inline BOOL style_control_vb_combo(HWND w)
{return style_control_vb_class(w,L"ThunderRT6ComboBox") || style_control_vb5_class(w,L"ThunderRT5ComboBox") || style_control_vb5_class(w,L"ThunderRT5DriveListBox");}
static inline BOOL style_control_vb_list(HWND w)
{return style_control_vb_class(w,L"ThunderRT6ListBox") || style_control_vb_class(w,L"ThunderRT6FileListBox") || style_control_vb_class(w,L"ThunderRT6DirListBox") || style_control_vb5_class(w,L"ThunderRT5ListBox") || style_control_vb5_class(w,L"ThunderRT5FileListBox") || style_control_vb5_class(w,L"ThunderRT5DirListBox");}
static inline BOOL style_control_vb_drive(HWND w)
{return style_control_vb5_class(w,L"ThunderRT5DriveListBox");}
static inline BOOL style_control_vb_directory(HWND w)
{return style_control_vb5_class(w,L"ThunderRT5DirListBox") || style_control_vb_class(w,L"ThunderRT6DirListBox");}
static inline BOOL style_control_vb_checklist_class(HWND w)
{return style_control_vb_class(w,L"ThunderRT6ListBox") || style_control_vb5_class(w,L"ThunderRT5ListBox");}
static inline BOOL style_control_vb_scroll(HWND w)
{return style_control_vb_class(w,L"ThunderRT6HScrollBar") || style_control_vb_class(w,L"ThunderRT6VScrollBar") || style_control_vb5_class(w,L"ThunderRT5HScrollBar") || style_control_vb5_class(w,L"ThunderRT5VScrollBar");}
static inline BOOL style_control_vb_button(HWND w)
{return style_control_vb_class(w,L"ThunderRT6CommandButton") || style_control_vb5_class(w,L"ThunderRT5CommandButton") || style_control_vb_checkbox(w) || style_control_vb_option(w);}
static inline BOOL style_control_class_is(HWND w,const WCHAR *expected)
{
    WCHAR name[64];
    if(!lstrcmpiW(expected,L"SysTabControl32") || !lstrcmpiW(expected,L"SysHeader32") || !lstrcmpiW(expected,L"SysTreeView32") || !lstrcmpiW(expected,L"msctls_progress32") || !lstrcmpiW(expected,L"msctls_trackbar32") || !lstrcmpiW(expected,L"msctls_updown32"))
        return style_control_protocol_is(w,expected);
    if(GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpiW(name,expected))return TRUE;
    if((!lstrcmpiW(expected,L"BUTTON") || !lstrcmpiW(expected,L"EDIT") ||
        !lstrcmpiW(expected,L"STATIC") || !lstrcmpiW(expected,L"COMBOBOX") ||
        !lstrcmpiW(expected,L"LISTBOX") || !lstrcmpiW(expected,L"SCROLLBAR")) &&
        style_control_native_superclass(w,expected))return TRUE;
    if(!lstrcmpiW(expected,L"BUTTON") && (style_control_vb_button(w) || (style_control_vb_class(w,L"ThunderRT6Frame") || style_control_vb5_class(w,L"ThunderRT5Frame"))))return TRUE;
    if(!lstrcmpiW(expected,L"COMBOBOX") && style_control_vb_combo(w))return TRUE;
    if(!lstrcmpiW(expected,L"LISTBOX") && style_control_vb_list(w))return TRUE;
    if(!lstrcmpiW(expected,L"SCROLLBAR") && style_control_vb_scroll(w))return TRUE;
    if(!lstrcmpiW(expected,L"EDIT"))return style_control_vb_class(w,L"ThunderRT6TextBox") || style_control_vb5_class(w,L"ThunderRT5TextBox") || style_control_protocol_is(w,expected);
    return style_control_protocol_is(w,expected);
}
static inline BOOL style_control_is_static(HWND w){return style_control_class_is(w,L"STATIC");}
static inline BOOL style_control_is_edit(HWND w){return style_control_class_is(w,L"EDIT");}
static inline BOOL style_control_is_combo(HWND w){return style_control_class_is(w,L"COMBOBOX");}
static inline BOOL style_control_is_scroll(HWND w){return style_control_class_is(w,L"SCROLLBAR");}
static inline BOOL style_control_is_link(HWND w)
{return style_control_class_is(w,L"SysLink");}
static inline BOOL style_control_is_tab(HWND w)
{return style_control_class_is(w,WC_TABCONTROLW);}
static inline BOOL style_control_is_header(HWND w){return style_control_class_is(w,WC_HEADERW);}
static inline BOOL style_control_is_separator(HWND w)
{
    DWORD type=(DWORD)GetWindowLongPtrW(w,GWL_STYLE)&SS_TYPEMASK;
    return style_control_is_static(w) && type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME;
}
static inline BOOL style_control_list_class(HWND w)
{WCHAR name[16];return style_control_class_is(w,L"LISTBOX") || (GetClassNameW(w,name,16) && !lstrcmpiW(name,L"ComboLBox"));}
static inline BOOL style_control_vb_frame(HWND w)
{return style_control_vb_class(w,L"ThunderRT6Frame") || style_control_vb5_class(w,L"ThunderRT5Frame");}
static inline UINT style_control_button_type(HWND w,DWORD style)
{return style_control_vb_frame(w)?BS_GROUPBOX:(style&BS_TYPEMASK);}
static inline BOOL style_control_vb_edit(HWND w)
{return style_control_vb_class(w,L"ThunderRT6TextBox") || style_control_vb5_class(w,L"ThunderRT5TextBox");}
#endif
