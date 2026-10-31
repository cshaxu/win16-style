#ifndef NTW16S_GLOBAL_REMOTE_H
#define NTW16S_GLOBAL_REMOTE_H
#include <windows.h>

/* Read-only resolution for a same-bitness target. The caller owns the process
 * handle and must keep the target alive while using the resulting address. */
DWORD ntw16s_remote_loadlibrary(HANDLE process,DWORD pid,void **address);
#endif
