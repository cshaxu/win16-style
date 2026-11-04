#include "native_combo_identity.h"
#include "combo_paint.h"
#include <oleacc.h>

/* The application owns owner-drawn swatches and text. Only the documented
 * native button rectangle is eligible for post-paint chrome. */
BOOL win16_control_color_identity(HWND w,const win16_control_native_combo_view *v,COMBOBOXINFO *info)
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
void win16_control_color_button(HWND w,const win16_control_native_combo_view *v,HDC dc,const RECT *dirty)
{
    COMBOBOXINFO info={.cbSize=sizeof(info)};
    if(!win16_control_color_identity(w,v,&info))return;
    BOOL pressed=SendMessageW(w,CB_GETDROPPEDSTATE,0,0)!=0;
    if(!*v->detached && IsWindow(w) && GetParent(w)==*v->parent)
        win16_control_native_combo_button(w,v->detached,dc,dirty,info.rcButton,pressed);
}
BOOL win16_control_date_identity(HWND w,const win16_control_native_combo_view *v,DATETIMEPICKERINFO *info)
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
void win16_control_date_button(HWND w,const win16_control_native_combo_view *v,HDC dc,const RECT *dirty)
{
    DATETIMEPICKERINFO info={.cbSize=sizeof(info)};
    if(win16_control_date_identity(w,v,&info) && !*v->detached && GetParent(w)==*v->parent)
        win16_control_native_combo_button(w,v->detached,dc,dirty,info.rcButton,(info.stateButton&STATE_SYSTEM_PRESSED)!=0);
}
