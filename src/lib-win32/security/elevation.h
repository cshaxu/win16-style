#ifndef WIN32_SECURITY_ELEVATION_H
#define WIN32_SECURITY_ELEVATION_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
/* error is required and receives the original Win32 error, or zero. */
BOOL win32_security_is_elevated(DWORD *error);
#ifdef __cplusplus
}
#endif
#endif
