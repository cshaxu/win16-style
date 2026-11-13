#include "lib-style/common/control_protocol.h"
#include "lib-style/common/control_vcl_spin.h"
#include <stdio.h>
#include <wchar.h>
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);WCHAR path[MAX_PATH];
    WNDCLASSW cls={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"StyleRuntimeClassTest"};
    if(!GetModuleFileNameW(instance,path,ARRAYSIZE(path)) || !RegisterClassW(&cls))return 1;
    WCHAR *base=wcsrchr(path,L'\\');base=base?base+1:path;
    HWND window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPED,0,0,20,20,NULL,NULL,instance,NULL);
    if(!window)return 2;
    BOOL ok=style_control_runtime_class(window,L"styleruntimeclasstest",base) &&
        !style_control_runtime_class(window,L"WrongClass",base) &&
        !style_control_runtime_class(window,L"StyleRuntimeClassTest",L"kernel32.dll") &&
        !style_control_runtime_class(window,L"StyleRuntimeClassTest",L"StyleMissingRuntime.dll") &&
        !style_control_vcl_spin_identity(window) && !style_control_vcl_modern_spin_shape(window) &&
        !style_control_vcl_spinedit_button(window);
    WNDCLASSW spin_edit={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"TSpinEdit"};
    WNDCLASSW spin_button={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"TSpinButton"};
    BOOL edit_class=RegisterClassW(&spin_edit)!=0,button_class=RegisterClassW(&spin_button)!=0;
    HWND edit=edit_class?CreateWindowW(spin_edit.lpszClassName,L"",WS_CHILD,0,0,20,20,window,NULL,instance,NULL):NULL;
    HWND button=edit && button_class?CreateWindowW(spin_button.lpszClassName,L"",WS_CHILD,0,0,10,10,edit,NULL,instance,NULL):NULL;
    ok=ok && button && style_control_vcl_modern_spin_shape(button);
    if(button){SetParent(button,window);ok=ok && !style_control_vcl_modern_spin_shape(button);DestroyWindow(button);}
    if(edit)DestroyWindow(edit);
    if(button_class)UnregisterClassW(spin_button.lpszClassName,instance);
    if(edit_class)UnregisterClassW(spin_edit.lpszClassName,instance);
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    if(!ok)return 3;
    puts("CONTROL_RUNTIME_OK name=1 case=1 owner=1 missing=1 cleanup=1");return 0;
}
