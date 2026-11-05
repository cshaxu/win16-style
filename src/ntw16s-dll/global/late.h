#ifndef NTW16S_GLOBAL_LATE_H
#define NTW16S_GLOBAL_LATE_H
#include <windows.h>
/* Called only after the target callback authenticates the active session. */
BOOL ntw16s_global_late_schedule(DWORD cookie);
void ntw16s_global_late_mark_ready(DWORD cookie);
#endif
