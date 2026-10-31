#ifndef NTW16S_GLOBAL_ATTACH_H
#define NTW16S_GLOBAL_ATTACH_H
#include <windows.h>
#include "global_policy.h"
#include "global_load.h"
#include "global_start.h"

typedef struct ntw16s_global_attach {
    ntw16s_global_identity identity;
    ntw16s_global_load load;
    ntw16s_global_start start;
    HANDLE process;
    HWND window;
    WCHAR *dll_path;
    DWORD cookie,phase,control_error;
} ntw16s_global_attach;

/* Each operation is nonblocking except for one bounded control message. The
 * caller retains tickets across ERROR_IO_PENDING and signals the session stop
 * event before close; no operation terminates a target process or thread. */
DWORD ntw16s_global_attach_begin(const ntw16s_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket);
DWORD ntw16s_global_attach_advance(ntw16s_global_attach *ticket,SECURITY_ATTRIBUTES *security);
DWORD ntw16s_global_attach_apply(ntw16s_global_attach *ticket);
DWORD ntw16s_global_attach_remove(ntw16s_global_attach *ticket);
DWORD ntw16s_global_attach_close(ntw16s_global_attach *ticket);
#endif
