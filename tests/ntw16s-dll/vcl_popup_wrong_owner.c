/* A different module deliberately creates a class owned by the executable. */
#include <windows.h>
__declspec(dllexport) HWND WINAPI wrong_owner(HINSTANCE instance)
{
    HWND w=CreateWindowExW(WS_EX_TOOLWINDOW,L"TPUtilWindow",L"",WS_POPUP,
        40,40,100,80,NULL,NULL,instance,NULL);
    return w;
}
