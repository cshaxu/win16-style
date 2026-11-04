#ifndef NTW16S_STARTUP_HOOKS_H
#define NTW16S_STARTUP_HOOKS_H
#include "../../lib-win32/process-hook/startup_hooks.h"
#define ntw16s_hooks_begin process_hook_startup_begin
#define ntw16s_hooks_attach process_hook_startup_attach
#define ntw16s_hooks_commit process_hook_startup_commit
#define ntw16s_hooks_abort process_hook_startup_abort
#endif
