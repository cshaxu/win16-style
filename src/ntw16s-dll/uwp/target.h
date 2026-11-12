#ifndef NTW16S_UWP_TARGET_H
#define NTW16S_UWP_TARGET_H
#include "lib-style/uwp/target.h"
/* Product coordinator, called outside the host UI thread. Caller initializes
 * COM before apply; handles are observations, not ownership or stable identity.
 * change revalidates the current frame/CoreWindow association and process ID.
 * Removal detaches the renderer; the host-pinned DLL is not unloaded. */
DWORD ntw16s_uwp_target_change(const style_uwp_target *target,PCWSTR dll_path,BOOL apply);
#endif
