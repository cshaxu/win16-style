#include "lib-style/common/control_protocol.h"
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
        !style_control_runtime_class(window,L"StyleRuntimeClassTest",L"StyleMissingRuntime.dll");
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    if(!ok)return 3;
    puts("CONTROL_RUNTIME_OK name=1 case=1 owner=1 missing=1 cleanup=1");return 0;
}
