/* Project-authored private PE import staging for newly created suspended targets.
 * Loader bootstrap restores directory metadata, but retains the allocation until
 * process exit because the loader can still hold pointers into its descriptors.
 */
#ifndef NTW16S_STAGING_H
#define NTW16S_STAGING_H
#include "injection.h"
#include "../../lib-win32/process-hook/staging.h"
#include "../../lib-win32/process-hook/helper.h"
void ntw16s_run_helper(void);
#endif
