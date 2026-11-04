/* Project-authored private PE import staging for newly created suspended targets.
 * Loader bootstrap restores directory metadata, but retains the allocation until
 * process exit because the loader can still hold pointers into its descriptors.
 */
#ifndef NTW16S_STAGING_H
#define NTW16S_STAGING_H
#include "injection.h"
#include "../../lib-win32/process-hook/staging.h"
#include "../../lib-win32/process-hook/helper.h"
#define ntw16s_stage_process process_hook_stage_process
#define ntw16s_stage_restore process_hook_stage_restore
DWORD ntw16s_stage_read(ntw16s_context *context,BOOL *found);
#define ntw16s_install_helper process_hook_install_helper
void ntw16s_run_helper(void);
#endif
