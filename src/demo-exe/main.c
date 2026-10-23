/* Adapted fixed-source W16Style entry: native application only. */
#include <windows.h>
int demo_run(HINSTANCE instance,int show);
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR command,int show)
{
    (void)previous;(void)command;
    if(!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))return (int)GetLastError();
    return demo_run(instance,show);
}
