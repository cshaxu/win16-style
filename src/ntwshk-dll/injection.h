#ifndef NTWSHK_INJECTION_H
#define NTWSHK_INJECTION_H
#include <windows.h>
#include <stdint.h>
#define NTWSHK_PATH_CAP 32752
#define NTWSHK_POLICY_STYLE 1u
#define NTWSHK_POLICY_PROPAGATE 2u
#define NTWSHK_POLICY_GLOBAL_SESSION 4u
#define NTWSHK_MODULE_CAP 64
#ifdef __cplusplus
extern "C" {
#endif
typedef struct ntwshk_packet {
    uint32_t bytes,version,machine,flags,directory_offset,directory_bytes,reserved0,reserved1;
} ntwshk_packet;
C_ASSERT(sizeof(ntwshk_packet)==32);
typedef struct ntwshk_context {DWORD flags,session_cookie;WCHAR directory[NTWSHK_PATH_CAP],module[NTWSHK_MODULE_CAP];} ntwshk_context;
typedef BOOL (WINAPI *ntwshk_create_w)(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,
    LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION);
extern const GUID ntwshk_payload_guid;
DWORD ntwshk_packet_parse(const void *data,DWORD bytes,DWORD machine,ntwshk_context *context);
DWORD ntwshk_context_read(ntwshk_context *context,BOOL *found);
DWORD ntwshk_context_paths(ntwshk_context *context,HMODULE module);
DWORD ntwshk_context_library(const ntwshk_context *context,WORD machine,WCHAR *path,size_t capacity);
/* Borrow only a newly created, uncommitted suspended target. Does not resume,
 * terminate, close it, or transfer handles. Creator owns rollback on failure. */
DWORD ntwshk_install(HANDLE child,const ntwshk_context *context,ntwshk_create_w create_helper);
#ifdef __cplusplus
}
#endif
#endif
