#ifndef RUN16S_WINDOW_TARGET_H
#define RUN16S_WINDOW_TARGET_H
#include <windows.h>
/* On success the caller owns *module and must release it with FreeLibrary. */
DWORD run16s_window_module_open(DWORD pid, PCWSTR directory, HMODULE *module, HOOKPROC *callback);
DWORD run16s_window_send(HWND window, UINT message);
DWORD run16s_window_attach(HWND window, DWORD pid, DWORD thread, PCWSTR directory);
#endif
