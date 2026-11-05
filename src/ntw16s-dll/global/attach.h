#ifndef NTW16S_GLOBAL_ATTACH_H
#define NTW16S_GLOBAL_ATTACH_H
#include <windows.h>
#include "ntw16s-dll/global/policy.h"
#include "ntw16s-dll/global/remote.h"
#include "ntw16s-dll/global/start.h"

typedef struct ntw16s_global_attach {
    win32_process_global_identity identity;
    win32_process_global_load load;
    win32_process_global_worker start;
    HANDLE process;
    HWND window;
    WCHAR *dll_path;
    DWORD cookie,phase,control_error;
} ntw16s_global_attach;

/* Each operation is nonblocking except for one bounded control message. The
 * caller retains tickets across ERROR_IO_PENDING and signals the session stop
 * event before close; no operation terminates a target process or thread. */
DWORD ntw16s_global_attach_begin(const win32_process_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket);
DWORD ntw16s_global_attach_begin_thread(const win32_process_global_policy *policy,DWORD pid,DWORD thread_id,
                                       WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket);
DWORD ntw16s_global_attach_advance(ntw16s_global_attach *ticket,SECURITY_ATTRIBUTES *security);
DWORD ntw16s_global_attach_apply(ntw16s_global_attach *ticket);
DWORD ntw16s_global_attach_remove(ntw16s_global_attach *ticket);
DWORD ntw16s_global_attach_close(ntw16s_global_attach *ticket);
#endif
