#ifndef STYLE_STARTUP_INSTALL_H
#define STYLE_STARTUP_INSTALL_H
#include "context.h"
#include "lib-win32/process-hook/helper.h"

/* Borrow only a newly created, uncommitted suspended target. Its creator
 * owns resume, termination, handles and rollback. Dependency is the caller's
 * shared-module basename; both modules are resolved in the selected package. */
DWORD style_startup_install(HANDLE,const style_startup_context *,PCWSTR,process_hook_create_w);
DWORD style_startup_run_helper(PCWSTR dependency);
#endif
