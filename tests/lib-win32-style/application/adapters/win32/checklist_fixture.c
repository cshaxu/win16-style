#include <windows.h>
static HINSTANCE module;
static WNDPROC native;
static UINT action;
static HWND next_parent;
static LRESULT CALLBACK list_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==LB_ITEMFROMPOINT){
        UINT now=action;action=0;
        if(now==1)EnableWindow(w,FALSE);
        else if(now==2)SetWindowLongPtrW(w,GWL_STYLE,GetWindowLongPtrW(w,GWL_STYLE)^WS_TABSTOP);
        else if(now==3)SetParent(w,next_parent);
    }
    return CallWindowProcW(native,w,m,a,b);
}
__declspec(dllexport) BOOL WINAPI ChecklistRegister(void)
{
    WNDCLASSW c;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)(UINT_PTR)ChecklistRegister,(HMODULE *)&module) || !GetClassInfoW(NULL,L"LISTBOX",&c))return FALSE;
    native=c.lpfnWndProc;c.lpfnWndProc=list_proc;c.hInstance=module;
    c.lpszClassName=L"ThunderRT6ListBox";c.style&=~CS_GLOBALCLASS;
    return RegisterClassW(&c)!=0;
}
__declspec(dllexport) void WINAPI ChecklistArm(UINT value,HWND parent)
{action=value;next_parent=parent;}
__declspec(dllexport) BOOL WINAPI ChecklistRelease(void)
{return UnregisterClassW(L"ThunderRT6ListBox",module);}
