#ifndef RUN16S_STARTUP_ERROR_H
#define RUN16S_STARTUP_ERROR_H
#include <windows.h>
#include <shellapi.h>
BOOL run16s_private_console(void);
BOOL run16s_legacy_image(PCWSTR image);
DWORD run16s_system_error(PCWSTR image,PCWSTR arguments,
    BOOL (WINAPI *execute)(SHELLEXECUTEINFOW *));
#endif
