#ifndef NTW16S_UWP_TARGET_H
#define NTW16S_UWP_TARGET_H
#include <windows.h>
/* Product coordinator, called outside the host UI thread. Caller initializes
 * COM before apply; handles are observations, not ownership or stable identity.
 * change revalidates the current frame/CoreWindow association and process ID.
 * Removal detaches the renderer; the host-pinned DLL is not unloaded. */
typedef struct ntw16s_uwp_target {DWORD pid;HWND core,frame;} ntw16s_uwp_target;
BOOL ntw16s_uwp_target_find(DWORD pid,ntw16s_uwp_target *target);
BOOL ntw16s_uwp_target_window(HWND window,ntw16s_uwp_target *target);
DWORD ntw16s_uwp_target_change(const ntw16s_uwp_target *target,PCWSTR dll_path,BOOL apply);
#endif
