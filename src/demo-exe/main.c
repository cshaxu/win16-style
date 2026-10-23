/* Adapted fixed-source W16Style entry: native application only. */
#include <windows.h>
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
int demo_run(HINSTANCE instance,int show);
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR command,int show)
{
    (void)previous;(void)command;
    if(!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))return (int)GetLastError();
    return demo_run(instance,show);
}
