#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <dwmapi.h>
#include <stdio.h>
#include "ntw16s-dll/attachment.h"
#include "ntw16s-dll/global/window.h"

static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefWindowProcW(w,m,a,b);}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName=L"Win16DwmRestoreTest"};
    if(!RegisterClassW(&cls))return 2;
    HWND window=CreateWindowW(cls.lpszClassName,L"Win16 DWM",WS_OVERLAPPEDWINDOW,80,80,420,260,NULL,NULL,instance,NULL);
    if(!window)return 2;
    ShowWindow(window,SW_SHOWNOACTIVATE);UpdateWindow(window);
    DwmFlush();
    printf("WIN16 WINDOW CONDITIONS style=%08lx ex=%08lx active=%d foreground=%d\n",
           (unsigned long)GetWindowLongW(window,GWL_STYLE),(unsigned long)GetWindowLongW(window,GWL_EXSTYLE),
           GetActiveWindow()==window,GetForegroundWindow()==window);
    BOOL before=FALSE,after=FALSE;
    BOOL observed=SUCCEEDED(DwmGetWindowAttribute(window,DWMWA_NCRENDERING_ENABLED,&before,sizeof(before)));
    BOOL attached=Ntw16sWindowAttach(window);
    MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    BOOL removed=attached && Ntw16sWindowRemove(window);
    DwmFlush();BOOL read=SUCCEEDED(DwmGetWindowAttribute(window,DWMWA_NCRENDERING_ENABLED,&after,sizeof(after)));
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    printf("WIN16 DWM RESTORE bits=%u observed=%d before=%d attached=%d removed=%d read=%d after=%d\n",
           (unsigned)(sizeof(void *)*8),observed,before,attached,removed,read,after);
    return observed && attached && removed && read && before==after?0:3;
}
