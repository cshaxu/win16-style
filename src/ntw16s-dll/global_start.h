#ifndef NTW16S_GLOBAL_START_H
#define NTW16S_GLOBAL_START_H
#include <windows.h>

#include "lib-win32/process-global/worker.h"
typedef win32_process_global_worker ntw16s_global_start;

DWORD ntw16s_global_start_begin(HANDLE process,DWORD pid,DWORD thread_id,DWORD cookie,
                                const WCHAR *dll_path,SECURITY_ATTRIBUTES *security,ntw16s_global_start *ticket);
DWORD ntw16s_global_start_ready(ntw16s_global_start *ticket,DWORD timeout);
DWORD ntw16s_global_start_close(ntw16s_global_start *ticket);
#endif
