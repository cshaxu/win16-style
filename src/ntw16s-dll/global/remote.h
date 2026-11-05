#ifndef NTW16S_GLOBAL_REMOTE_H
#define NTW16S_GLOBAL_REMOTE_H
#include <windows.h>
#include "lib-win32/process-global/load.h"

DWORD ntw16s_global_load_begin(HANDLE,DWORD,const WCHAR *,win32_process_global_load *);
DWORD ntw16s_global_load_finish(win32_process_global_load *,DWORD,void **);
/* Resolves the named internal worker from the target's own matching DLL image;
 * this never loads, writes or executes code in the target. */
DWORD ntw16s_remote_global_thread(HANDLE process,DWORD pid,const WCHAR *path,void **address);
#endif
