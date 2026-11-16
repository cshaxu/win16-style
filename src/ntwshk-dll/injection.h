#ifndef NTWSHK_INJECTION_H
#define NTWSHK_INJECTION_H
#include "lib-win32-style/access/startup/context.h"
#include "lib-win32/process-hook/helper.h"

/* Existing product ABI; the implementation and state are shared components. */
typedef style_startup_packet ntwshk_packet;
typedef style_startup_context ntwshk_context;
typedef process_hook_create_w ntwshk_create_w;
#define NTWSHK_PATH_CAP STYLE_STARTUP_PATH_CAP
#define NTWSHK_MODULE_CAP STYLE_STARTUP_MODULE_CAP
#define NTWSHK_POLICY_STYLE STYLE_STARTUP_POLICY_STYLE
#define NTWSHK_POLICY_PROPAGATE STYLE_STARTUP_POLICY_PROPAGATE
#define NTWSHK_POLICY_GLOBAL_SESSION STYLE_STARTUP_POLICY_GLOBAL_SESSION
DWORD ntwshk_packet_parse(const void *,DWORD,DWORD,ntwshk_context *);
DWORD ntwshk_context_read(ntwshk_context *,BOOL *);
DWORD ntwshk_context_paths(ntwshk_context *,HMODULE);
DWORD ntwshk_context_library(const ntwshk_context *,WORD,WCHAR *,size_t);
DWORD ntwshk_install(HANDLE,const ntwshk_context *,ntwshk_create_w);
#endif
