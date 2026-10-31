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
DWORD ntw16s_late_patch_discard(ntw16s_late_patch *);
#endif
