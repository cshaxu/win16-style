#include "control_native_combo.h"

BOOL style_control_color_identity(HWND w,const style_control_native_combo_view *v,COMBOBOXINFO *info)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    WNDCLASSEXW native={.cbSize=sizeof(native)};RECT client,overlap;
    if(*v->detached || !IsWindow(w) || !*v->owner ||
       (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=*v->owner ||
       (style&3)!=CBS_DROPDOWNLIST || !(style&CBS_OWNERDRAWFIXED) || !(style&CBS_HASSTRINGS) ||
       (style&CBS_OWNERDRAWVARIABLE) || (ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
       !GetParent(w) || (*v->parent && GetParent(w)!=*v->parent) ||
       GetWindowThreadProcessId(GetParent(w),NULL)!=GetCurrentThreadId() ||
       !GetClassInfoExW(NULL,L"COMBOBOX",&native) ||
       GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)native.cbWndExtra ||
       !GetComboBoxInfo(w,info) || *v->detached || info->hwndCombo!=w || info->hwndItem!=w ||
       !info->hwndList || GetWindowThreadProcessId(info->hwndList,NULL)!=GetCurrentThreadId() ||
       !GetClientRect(w,&client) || info->rcButton.left<client.left || info->rcButton.top<client.top ||
       info->rcButton.right>client.right || info->rcButton.bottom>client.bottom || IsRectEmpty(&info->rcButton) ||
       IntersectRect(&overlap,&info->rcButton,&info->rcItem))return FALSE;
    LRESULT count=SendMessageW(w,CB_GETCOUNT,0,0);if(*v->detached || count<0)return FALSE;
    LRESULT selected=SendMessageW(w,CB_GETCURSEL,0,0);
    return !*v->detached && IsWindow(w) && (selected==CB_ERR || (selected>=0 && selected<count));
}
BOOL style_control_date_identity(HWND w,const style_control_native_combo_view *v,DATETIMEPICKERINFO *info)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    WNDCLASSEXW native={.cbSize=sizeof(native)};RECT client;SYSTEMTIME value={0};
    if(*v->detached || !IsWindow(w) || !*v->owner ||
       (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=*v->owner ||
       (style&(DTS_UPDOWN|DTS_SHOWNONE|DTS_APPCANPARSE)) || (ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
       !GetParent(w) || (*v->parent && GetParent(w)!=*v->parent) ||
       GetWindowThreadProcessId(GetParent(w),NULL)!=GetCurrentThreadId() ||
       !GetClassInfoExW(NULL,DATETIMEPICK_CLASSW,&native) ||
       GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)native.cbWndExtra ||
       !SendMessageW(w,DTM_GETDATETIMEPICKERINFO,0,(LPARAM)info) || *v->detached ||
       info->hwndEdit || info->hwndUD || !IsRectEmpty(&info->rcCheck) ||
       !GetClientRect(w,&client) || info->rcButton.left<client.left || info->rcButton.top<client.top ||
       info->rcButton.right>client.right || info->rcButton.bottom>client.bottom || IsRectEmpty(&info->rcButton))return FALSE;
    LRESULT status=SendMessageW(w,DTM_GETSYSTEMTIME,0,(LPARAM)&value);
    return !*v->detached && IsWindow(w) && status==GDT_VALID;
}

/* Only the actual native EDIT named by this runtime combo's public relation.
 * No extra window search or transient HFONT replacement in the application. */
BOOL style_control_combo_edit(HWND w,BOOL (*accept_parent)(HWND))
{
    HWND parent=GetParent(w);WCHAR name[32];COMBOBOXINFO info={.cbSize=sizeof(info)};
    WNDCLASSEXW native={.cbSize=sizeof(native)};HMODULE owner=NULL;
    if(!parent || !accept_parent || !accept_parent(parent) || GetWindowThreadProcessId(parent,NULL)!=GetCurrentThreadId() ||
        GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpiW(name,L"EDIT") ||
        !GetClassInfoExW(NULL,L"EDIT",&native) ||
        GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)native.cbWndExtra)return FALSE;
    BOOL resolved=GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)(UINT_PTR)native.lpfnWndProc,&owner);
    HMODULE registered=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(!resolved || owner!=registered){
        /* The class descriptor can expose an A/W dispatch thunk (for example
         * in NTDLL) rather than the HWND's native USER class procedure. */
        WNDPROC actual=(WNDPROC)(UINT_PTR)GetClassLongPtrW(w,GCLP_WNDPROC);
        if(native.hInstance || registered!=GetModuleHandleW(L"USER32.DLL"))return FALSE;
        if(actual!=native.lpfnWndProc &&
            (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (const WCHAR *)(UINT_PTR)actual,&owner) || owner!=registered))return FALSE;
    }
    return GetComboBoxInfo(parent,&info) && info.hwndCombo==parent && info.hwndItem==w;
}

BOOL style_control_dialog_combo_valid(HWND w,HWND parent,BOOL (*is_combo)(HWND))
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    int id=GetDlgCtrlID(w);
    return w && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() &&
        is_combo && is_combo(w) && id>=1136 && id<=1140 && (style&CBS_OWNERDRAWFIXED) &&
        (style&CBS_HASSTRINGS) && !(style&CBS_OWNERDRAWVARIABLE);
}
