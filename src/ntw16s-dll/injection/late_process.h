#ifndef NTW16S_LATE_PROCESS_H
#define NTW16S_LATE_PROCESS_H
#include <windows.h>
/* Experimental internal initializer, not yet called by the global manager.
 * A caller must supply the validated absolute directory containing both DLLs.
 * Already-initialized contexts are left alone. */
DWORD ntw16s_late_process_init(PCWSTR directory,DWORD session_cookie);
#endif
