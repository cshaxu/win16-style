#ifndef NTW16S_GLOBAL_SESSION_H
#define NTW16S_GLOBAL_SESSION_H
#include <windows.h>
#include "lib-win32-style/access/global/session.h"
#include "lib-win32-style/access/global/start.h"
#include "lib-win32-style/access/global/attach.h"
#include "lib-win32-style/access/global/manager.h"
#include "lib-win32/process-global/identity.h"
#include "lib-win32/process-global/module.h"
#define NTW16S_GLOBAL_MAGIC 0x47363157u
#define NTW16S_GLOBAL_VERSION 2u
#define NTW16S_GLOBAL_STYLE_16 0u
#define NTW16S_GLOBAL_STYLE_2K 1u
#define NTW16S_GLOBAL_ACTIVE STYLE_GLOBAL_ACTIVE
#define NTW16S_GLOBAL_STOPPING STYLE_GLOBAL_STOPPING
#define NTW16S_GLOBAL_APPLY 1
#define NTW16S_GLOBAL_REMOVE 2
#define NTW16S_GLOBAL_QUERY 3
#define NTW16S_GLOBAL_ATTACHED 0x1631a55au
#define NTW16S_GLOBAL_NAME_CAP STYLE_GLOBAL_NAME_CAP
#define NTW16S_GLOBAL_TRACE_PREFIX L"Win16S-ETW-"
#define NTW16S_GLOBAL_CONTROL L"Win16Style.Global.Control.1"
typedef style_global_session ntw16s_global_session;
extern style_global_namespace ntw16s_global_namespace;
extern const style_global_start_config ntw16s_global_start_config;
extern const style_global_attach_config ntw16s_global_attach_config;
extern const style_global_manager_config ntw16s_global_manager_config;
BOOL ntw16s_global_policy_active(DWORD cookie);
BOOL ntw16s_global_policy_style(DWORD cookie,DWORD style);
#endif
