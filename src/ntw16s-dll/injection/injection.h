#ifndef NTW16S_INJECTION_H
#define NTW16S_INJECTION_H
#include <windows.h>
#include <stdint.h>
#define NTW16S_PATH_CAP 32752
#define NTW16S_POLICY_STYLE 1u
#define NTW16S_POLICY_PROPAGATE 2u
#define NTW16S_POLICY_GLOBAL_SESSION 4u
#ifdef __cplusplus
extern "C" {
#endif
typedef struct ntw16s_packet {
    uint32_t bytes,version,machine,flags,directory_offset,directory_bytes,reserved0,reserved1;
} ntw16s_packet;
C_ASSERT(sizeof(ntw16s_packet)==32);
typedef struct ntw16s_context {DWORD flags,session_cookie;WCHAR directory[NTW16S_PATH_CAP];} ntw16s_context;
typedef BOOL (WINAPI *ntw16s_create_w)(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,
    LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION);
extern const GUID ntw16s_payload_guid;
DWORD ntw16s_packet_parse(const void *data,DWORD bytes,DWORD machine,ntw16s_context *context);
DWORD ntw16s_context_read(ntw16s_context *context,BOOL *found);
DWORD ntw16s_context_paths(ntw16s_context *context,HMODULE module);
DWORD ntw16s_process_machine(HANDLE child,WORD *machine);
DWORD ntw16s_validate_image(PCWSTR path,WORD machine,BOOL dll);
/* Optional output from the same validated PE header, set only on success. */
DWORD ntw16s_validate_image_ex(PCWSTR path,WORD machine,BOOL dll,WORD *subsystem);
/* Borrow only a newly created, uncommitted suspended target. Does not resume,
 * terminate, close it, or transfer handles. Creator owns rollback on failure. */
DWORD ntw16s_install(HANDLE child,const ntw16s_context *context,ntw16s_create_w create_helper);
#ifdef __cplusplus
}
#endif
#endif
