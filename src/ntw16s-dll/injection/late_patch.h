#ifndef NTW16S_LATE_PATCH_H
#define NTW16S_LATE_PATCH_H
#include <windows.h>
#include "late_guard.h"
/* Experimental, single-owner transaction. The MinHook buffer allocator must
 * already be initialized and must not be shared with another hook backend.
 * Installed trampolines are retained; removal is not implemented. */
typedef struct ntw16s_late_patch {
    void *target,*replacement,*trampoline,*slot;
    BYTE backup[7]; BOOL above,installed;
    ntw16s_late_guard guard;
} ntw16s_late_patch;
DWORD ntw16s_late_patch_prepare(ntw16s_late_patch *,void *,void *);
DWORD ntw16s_late_patch_install(ntw16s_late_patch *);
/* A late error may occur after all patches were written: inspect each
 * installed flag and retain their trampolines. If guard retains handles,
 * retry guard_end before touching or discarding any batch member. */
DWORD ntw16s_late_patch_install_batch(ntw16s_late_guard *,ntw16s_late_patch **,DWORD);
DWORD ntw16s_late_patch_discard(ntw16s_late_patch *);
#ifdef NTW16S_LATE_GUARD_TEST
void ntw16s_late_patch_test_fail_protect_after(DWORD);
#endif
#endif
