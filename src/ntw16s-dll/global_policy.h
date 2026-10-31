#ifndef NTW16S_GLOBAL_POLICY_H
#define NTW16S_GLOBAL_POLICY_H
#include <windows.h>

typedef struct ntw16s_global_policy {
    BYTE sid[SECURITY_MAX_SID_SIZE];
    DWORD sid_bytes,session;
} ntw16s_global_policy;
typedef struct ntw16s_global_identity {
    DWORD pid,thread_id,process_birth_low,process_birth_high,thread_birth_low,thread_birth_high;
    WORD machine;
} ntw16s_global_identity;

BOOL ntw16s_global_image_allowed(const WCHAR *image,const WCHAR *windows);
DWORD ntw16s_global_policy_init(ntw16s_global_policy *policy);
DWORD ntw16s_global_policy_inspect(const ntw16s_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                   WORD expected_machine,ntw16s_global_identity *identity);
/* For startup discovery without an HWND, require the target thread to belong
 * to the caller's desktop as well as the normal process identity checks. */
DWORD ntw16s_global_policy_inspect_thread(const ntw16s_global_policy *policy,DWORD pid,DWORD thread_id,
                                          WORD expected_machine,ntw16s_global_identity *identity);
/* Revalidate the retained process object, thread birth and HWND immediately
 * before any target mutation; the caller owns the process handle. */
DWORD ntw16s_global_policy_revalidate(HANDLE process,HWND window,const ntw16s_global_identity *identity);
DWORD ntw16s_global_policy_revalidate_thread(HANDLE process,const ntw16s_global_identity *identity);
#endif
