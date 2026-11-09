#ifndef RUN16S_WINDOW_TARGET_H
#define RUN16S_WINDOW_TARGET_H
#include <windows.h>
DWORD run16s_window_module_compatible(DWORD pid, HMODULE module, HOOKPROC callback, PCWSTR path);
DWORD run16s_window_attach(HWND window, DWORD pid, DWORD thread, PCWSTR directory);
#endif
