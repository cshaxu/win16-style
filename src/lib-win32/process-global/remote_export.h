#ifndef WIN32_PROCESS_GLOBAL_REMOTE_EXPORT_H
#define WIN32_PROCESS_GLOBAL_REMOTE_EXPORT_H
#include <windows.h>
/* Resolve a non-forwarded, executable named export in a verified target module. */
DWORD win32_process_global_remote_export(HANDLE process,DWORD pid,const WCHAR *path,const WCHAR *expected_name,const char *export_name,void **address);
#endif
