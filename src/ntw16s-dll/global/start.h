#ifndef NTW16S_GLOBAL_START_H
#define NTW16S_GLOBAL_START_H
#include <windows.h>

#include "lib-win32/process-global/worker.h"

DWORD ntw16s_global_start_begin(HANDLE process,DWORD pid,DWORD thread_id,DWORD cookie,
                                const WCHAR *dll_path,SECURITY_ATTRIBUTES *security,win32_process_global_worker *ticket);
#endif
