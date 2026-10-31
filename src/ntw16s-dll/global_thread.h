#ifndef NTW16S_GLOBAL_THREAD_H
#define NTW16S_GLOBAL_THREAD_H
#include <windows.h>

/* The caller supplies an immutable, same-bitness packet in the target process.
 * This is an internal prototype, not a public ABI or an injection interface. */
typedef struct ntw16s_global_thread_packet {
    DWORD bytes, version, thread_id, cookie;
} ntw16s_global_thread_packet;

DWORD WINAPI ntw16s_global_thread_worker(void *parameter);
#endif
