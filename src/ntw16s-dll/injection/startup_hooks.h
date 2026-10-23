/* Private startup-only C patch transaction. Call once from the newly staged
 * process loader before application entry; never attach to a running process.
 * Original pointers remain usable trampolines for the process lifetime.
 */
#ifndef NTW16S_STARTUP_HOOKS_H
#define NTW16S_STARTUP_HOOKS_H
#include <windows.h>
DWORD ntw16s_hooks_begin(void);
/* Slot holds a Win32 function pointer. Byte copies avoid C aliasing violations
 * from writing a function-pointer object through void **. */
DWORD ntw16s_hooks_attach(void *original_slot,void *replacement);
DWORD ntw16s_hooks_commit(void);
void ntw16s_hooks_abort(void);
#endif
