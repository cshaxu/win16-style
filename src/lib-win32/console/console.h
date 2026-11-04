#ifndef WIN32_CONSOLE_H
#define WIN32_CONSOLE_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
void win32_console_rebind_stderr(void);
#ifdef __cplusplus
}
#endif
#endif
