#ifndef NTW16S_GLOBAL_POLICY_H
#define NTW16S_GLOBAL_POLICY_H
#include <windows.h>

#include "lib-win32/process-global/policy.h"

BOOL ntw16s_global_image_allowed(const WCHAR *image,const WCHAR *windows);
DWORD ntw16s_global_policy_inspect(const win32_process_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                   WORD expected_machine,win32_process_global_identity *identity);
/* For startup discovery without an HWND, require the target thread to belong
 * to the caller's desktop as well as the normal process identity checks. */
DWORD ntw16s_global_policy_inspect_thread(const win32_process_global_policy *policy,DWORD pid,DWORD thread_id,
                                          WORD expected_machine,win32_process_global_identity *identity);
#endif
