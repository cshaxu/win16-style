#ifndef NTW16S_CONTROL_CLASS_H
#define NTW16S_CONTROL_CLASS_H
#include <windows.h>
/* Private framework classification. A name alone is insufficient: only the
 * exact intrinsic classes registered by the loaded VB6 runtime are aliases.
 * No LoadLibrary, object-layout inspection or application-name special case. */
static inline BOOL ntw16s_vb_class(HWND w,const WCHAR *name)
{
    WCHAR actual[64];HMODULE runtime=GetModuleHandleW(L"MSVBVM60.DLL");
    return runtime && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==runtime &&
        GetClassNameW(w,actual,ARRAYSIZE(actual)) && !lstrcmpiW(actual,name);
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
    if(GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpiW(name,expected))return TRUE;
    if((!lstrcmpiW(expected,L"BUTTON") || !lstrcmpiW(expected,L"EDIT") ||
        !lstrcmpiW(expected,L"STATIC") || !lstrcmpiW(expected,L"COMBOBOX") ||
        !lstrcmpiW(expected,L"LISTBOX") || !lstrcmpiW(expected,L"SCROLLBAR")) &&
        ntw16s_native_superclass(w,expected))return TRUE;
    if(!lstrcmpiW(expected,L"BUTTON"))return ntw16s_vb_class(w,L"ThunderRT6CommandButton") ||
        ntw16s_vb_class(w,L"ThunderRT6Frame");
    if(!lstrcmpiW(expected,L"EDIT"))return ntw16s_vb_class(w,L"ThunderRT6TextBox");
    return FALSE;
}
static inline BOOL ntw16s_vb_frame(HWND w)
{return ntw16s_vb_class(w,L"ThunderRT6Frame");}
static inline BOOL ntw16s_vb_edit(HWND w)
{return ntw16s_vb_class(w,L"ThunderRT6TextBox");}
static inline UINT ntw16s_button_type(HWND w,DWORD style)
{return ntw16s_vb_frame(w)?BS_GROUPBOX:(style&BS_TYPEMASK);}
#endif
