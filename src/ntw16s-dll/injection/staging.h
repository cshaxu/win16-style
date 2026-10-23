/* Project-authored private PE import staging for newly created suspended targets.
 * Loader bootstrap restores directory metadata, but retains the allocation until
 * process exit because the loader can still hold pointers into its descriptors.
 */
#ifndef NTW16S_STAGING_H
#define NTW16S_STAGING_H
#include "injection.h"
DWORD ntw16s_stage_process(HANDLE child,const char *dll,const void *packet,DWORD bytes);
DWORD ntw16s_stage_read(ntw16s_context *context,BOOL *found);
DWORD ntw16s_stage_restore(void);
DWORD ntw16s_install_helper(HANDLE child,PCWSTR dll,const void *packet,DWORD bytes,ntw16s_create_w create);
void ntw16s_run_helper(void);
#endif
