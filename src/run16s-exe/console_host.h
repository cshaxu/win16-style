#ifndef RUN16S_CONSOLE_HOST_H
#define RUN16S_CONSOLE_HOST_H
#include <windows.h>

BOOL run16s_console_host_image(PCWSTR image);
DWORD run16s_console_host_attach(HANDLE process, PCWSTR directory);
DWORD run16s_console_host_attach_current(PCWSTR directory);
#endif
