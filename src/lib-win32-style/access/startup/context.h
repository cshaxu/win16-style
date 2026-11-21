#ifndef STYLE_STARTUP_CONTEXT_H
#define STYLE_STARTUP_CONTEXT_H
#include <windows.h>
#include <stdint.h>
#define STYLE_STARTUP_PATH_CAP 32752
#define STYLE_STARTUP_POLICY_STYLE 1u
#define STYLE_STARTUP_POLICY_PROPAGATE 2u
#define STYLE_STARTUP_POLICY_GLOBAL_SESSION 4u
#define STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE 8u
#define STYLE_STARTUP_PROFILE_SHIFT 4u
#define STYLE_STARTUP_POLICY_MASK 15u
#define STYLE_STARTUP_MODULE_CAP 64
#ifdef __cplusplus
extern "C" {
#endif
typedef struct style_startup_packet {
    uint32_t bytes,version,machine,flags,directory_offset,directory_bytes,reserved0,reserved1;
} style_startup_packet;
C_ASSERT(sizeof(style_startup_packet)==32);
typedef struct style_startup_selection {
    uint32_t theme_id,adapter;
} style_startup_selection;
C_ASSERT(sizeof(style_startup_selection)==8);
/* Decode or update the selection stored in the existing context ABI.
 * The caller supplies a validated wire-domain ID and adapter. */
static inline style_startup_selection style_startup_get_selection(DWORD flags)
{
    return (style_startup_selection){flags>>STYLE_STARTUP_PROFILE_SHIFT,
        !!(flags&STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE)};
}
static inline DWORD style_startup_with_selection(DWORD flags,style_startup_selection selection)
{
    return (flags&(STYLE_STARTUP_POLICY_MASK&~STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE)) |
        (selection.theme_id<<STYLE_STARTUP_PROFILE_SHIFT) |
        (selection.adapter?STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE:0u);
}
#define STYLE_STARTUP_PACKET_VERSION 4u
typedef struct style_startup_context {DWORD flags,session_cookie;WCHAR directory[STYLE_STARTUP_PATH_CAP],module[STYLE_STARTUP_MODULE_CAP];} style_startup_context;
DWORD style_startup_packet_parse(const void *data,DWORD bytes,DWORD machine,style_startup_context *context);
DWORD style_startup_read(style_startup_context *context,BOOL *found);
DWORD style_startup_paths(style_startup_context *context,HMODULE module);
DWORD style_startup_module_path(const style_startup_context *context,WORD machine,PCWSTR module,WCHAR *path,size_t capacity);
DWORD style_startup_library(const style_startup_context *context,WORD machine,WCHAR *path,size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
