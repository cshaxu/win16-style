#ifndef NTW16S_GLOBAL_REMOTE_H
#define NTW16S_GLOBAL_REMOTE_H
#include <windows.h>

/* Read-only resolution for a same-bitness target. The caller owns the process
 * handle and must keep the target alive while using the resulting address. */
DWORD ntw16s_remote_loadlibrary(HANDLE process,DWORD pid,void **address);
/* Resolves the named internal worker from the target's own matching DLL image;
 * this never loads, writes or executes code in the target. */
DWORD ntw16s_remote_global_thread(HANDLE process,DWORD pid,const WCHAR *path,void **address);
#endif
