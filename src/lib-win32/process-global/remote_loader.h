#ifndef WIN32_PROCESS_GLOBAL_REMOTE_LOADER_H
#define WIN32_PROCESS_GLOBAL_REMOTE_LOADER_H
#include <windows.h>
/* Resolves and verifies the same-architecture system loader in the target. */
DWORD win32_process_global_remote_loadlibrary(HANDLE process,DWORD pid,void **address);
#endif
