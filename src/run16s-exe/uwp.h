#ifndef RUN16S_UWP_H
#define RUN16S_UWP_H
#include <windows.h>
/* Optional PID is set only after confirming activation created a new process,
 * including when subsequent frame installation fails. Caller does not own it. */
DWORD run16s_uwp_activate(PCWSTR app_id,DWORD *created_pid);
#endif
